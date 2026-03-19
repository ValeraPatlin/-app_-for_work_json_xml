#ifndef TABLE_MODEL_H
#define TABLE_MODEL_H


#include <QAbstractTableModel>
#include <QList>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <QDebug>


struct DbRow
{
    int id;
    QString jsonData;
};

class TableModel final : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TableModel(QObject *parent = nullptr);
    ~TableModel() = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void clearModel();
    void setData(const QJsonArray& dataArray);

signals:
    void updateData(const int id, const QJsonObject& jsonObj) const;

public slots:
    void updateModelData(const QJsonArray& new_data);

private:
    QStringList columnHeaders;
    QList<DbRow> databaseRows;
};

#endif // TABLE_MODEL_H
