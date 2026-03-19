#include "table_model.h"

TableModel::TableModel(QObject *parent)
    : QAbstractTableModel{parent}
{
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return databaseRows.size();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return columnHeaders.size();
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= databaseRows.size())
    {
        return QVariant();
    }

    const DbRow& rowData = databaseRows.at(index.row());
    QString currentHeader = columnHeaders.value(index.column());

    if (currentHeader == "ID")
    {
        if (role == Qt::DisplayRole)
        {
            return rowData.id;
        }
    }
    else
    {
        if (role == Qt::DisplayRole)
        {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(rowData.jsonData.toUtf8(), &parseError);

            if (doc.isObject())
            {
                QJsonObject jsonObject = doc.object();

                if (jsonObject.contains(currentHeader))
                {
                    QJsonValue value = jsonObject.value(currentHeader);

                    if (value.isBool())
                        return value.toBool();
                    if (value.isString())
                        return value.toString();

                    if (value.isArray() || value.isObject())
                        return QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact);

                    return QVariant();
                }
                else
                {
                    return "N/A";
                }
            }
            else
            {
                qWarning() << "Failed to parse JSON for row " << rowData.id
                           << " : " << parseError.errorString();
                return "Parse Error";
            }
        }
    }

    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal)
    {
        if (section >= 0 && section < columnHeaders.size())
        {
            return columnHeaders.at(section);
        }
    }
    return QVariant();
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags baseFlags = QAbstractTableModel::flags(index);

    if (index.isValid() && index.column() != 0)
    {
        return baseFlags | Qt::ItemIsEditable;
    }
    return baseFlags;
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole)
    {
        if (!index.isValid() || !flags(index).testFlag(Qt::ItemIsEditable))
        {
            return false;
        }

        QVariant newData = value;

        if (index.row() < databaseRows.size() && index.column() < columnHeaders.size())
        {
            QString column = columnHeaders.value(index.column());

            QJsonDocument doc = QJsonDocument::fromJson(databaseRows.at(index.row()).jsonData.toUtf8());

            if (doc.isNull() || !doc.isObject())
            {
                qWarning() << "Invalid json document";
                return false;
            }

            QJsonObject jsonObj = doc.object();

            if (jsonObj.contains(column))
            {
                QJsonValue value = jsonObj.value(column);

                if (value.isBool())
                    jsonObj[columnHeaders.value(index.column())] = QJsonValue(newData.toBool());
                if (value.isString())
                    jsonObj[columnHeaders.value(index.column())] = QJsonValue(newData.toString());
                if (value.isArray())
                    jsonObj[columnHeaders.value(index.column())] = QJsonValue(newData.toJsonArray());

                emit updateData(databaseRows.at(index.row()).id, jsonObj);
            }
            else
            {
                qWarning() << "Error! Invalid key: " << columnHeaders.value(index.column());
            }
        }
        else
        {
            return false;
        }
        emit dataChanged(index, index, { Qt::EditRole, Qt::DisplayRole });
        return true;
    }
    return false;
}


void TableModel::clearModel()
{
    beginResetModel();
    databaseRows.clear();
    columnHeaders.clear();
    endResetModel();
}


void TableModel::setData(const QJsonArray &dataArray)
{
    beginResetModel();
    databaseRows.clear();
    columnHeaders.clear();

    if (!dataArray.isEmpty())
    {
        if (dataArray.first().isObject())
        {
            QJsonObject firstObject = dataArray.first().toObject();
            columnHeaders.append(firstObject.keys());
        }
        else
        {
            qWarning() << "First element in dataArray is not a JSON object";
            columnHeaders << "JSON Data";
        }

        for (const QJsonValue& value : dataArray)
        {
            if (value.isObject())
            {
                QJsonObject jsonObject = value.toObject();
                DbRow row;

                if (jsonObject.contains("ID"))
                {
                    row.id = jsonObject.value("ID").toInt();
                    jsonObject.remove("ID");
                }
                else
                {
                    row.id = -1;
                    qWarning() << "JSON object is missing 'id' key. Using -1";
                }

                QJsonDocument doc(jsonObject);
                row.jsonData = doc.toJson(QJsonDocument::Compact);
                databaseRows.append(row);
            }
            else
            {
                qWarning() << "Skipping non-object element in data array";
            }
        }
    }
    else
    {
        columnHeaders << "ID";
    }
    endResetModel();
}

void TableModel::updateModelData(const QJsonArray &new_data)
{
    databaseRows.clear();
    setData(new_data);
}
