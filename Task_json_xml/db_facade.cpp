#include "db_facade.h"

DbFacade::DbFacade(const QString &dbName, QObject *parent)
    : QObject{parent}, nameDb{dbName}
{
    database = QSqlDatabase::addDatabase("QSQLITE", "dbConnection");
    database.setDatabaseName(nameDb);
    openDatabase();
}

DbFacade::~DbFacade()
{
    closeDatabase();
}

bool DbFacade::openDatabase()
{
    if (!database.isOpen())
    {
        if (!database.open())
        {
            qCritical() << "Failed to open database: " << database.lastError().text();
            return false;
        }
        qDebug() << "Database opened successfully";

        QSqlQuery query(database);

        if (!query.exec("CREATE TABLE IF NOT EXISTS my_data_table ("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "data TEXT);"))
        {
            qCritical() << "Failed to create table: " << query.lastError().text();
            return false;
        }
        qDebug() << "Table created or already exists";

        return true;
    }
    return true;
}

void DbFacade::closeDatabase()
{
    if (database.isOpen())
    {
        database.close();
        qDebug() << "Database closed";

        if (QSqlDatabase::contains(database.connectionName()))
        {
            QSqlDatabase::removeDatabase(database.connectionName());
        }
    }
}


QJsonArray DbFacade::loadAllData() const
{
    QJsonArray dataArray;

    QSqlQuery query(database);
    if (!query.exec("SELECT id, data FROM my_data_table;"))
    {
        qCritical() << "Failed to create table: " << query.lastError().text();
    }

    while (query.next())
    {
        int id = query.value(0).toInt();
        QString jsonDataString = query.value(1).toString();

        QJsonParseError parseError;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonDataString.toUtf8(), &parseError);

        if (parseError.error != QJsonParseError::NoError)
        {
            qWarning() << "Failed to parse JSON for ID" << id << ":" << parseError.errorString();
            continue;
        }

        if (jsonDocument.isObject())
        {
            QJsonObject jsonObject = jsonDocument.object();
            jsonObject["ID"] = id;
            dataArray.append(jsonObject);
        }
        else
        {
            qWarning() << "JSON data for ID" << id << "is not an object";
            QJsonObject recordWithError;
            recordWithError["ID"] = id;
            recordWithError["Error"] = "JSON is not an object";
            dataArray.append(recordWithError);
        }
    }
    return dataArray;
}

QString DbFacade::loadData(const int id) const
{
    QSqlQuery query(database);
    query.prepare("SELECT data FROM my_data_table WHERE id = :id;");
    query.bindValue(":id", id);

    if (query.exec())
    {
        while (query.next())
        {
            return query.value(0).toString();
        }
    }
    return QString();
}

bool DbFacade::insertData(const QJsonArray &dataArray)
{
    if (dataArray.isEmpty())
    {
        qDebug() << "No data to insert";
        return true;
    }
    database.transaction();

    QSqlQuery query(database);
    query.prepare("INSERT INTO my_data_table (data) VALUES (:jsonData);");

    bool success = true;

    for (const QJsonValue& value : dataArray)
    {
        if (!value.isObject())
        {
            qWarning() << "Skipping non-object in insertData";
            continue;
        }

        QJsonObject jsonObject = value.toObject();
        QJsonDocument doc(jsonObject);
        QString jsonDataString = doc.toJson(QJsonDocument::Compact);

        query.bindValue(":jsonData", jsonDataString);

        if (!query.exec())
        {
            qWarning() << "Data insertion failed for entry: "
                       << jsonDataString << query.lastError().text();
            success = false;
        }
    }

    if (success)
    {
        database.commit();
        qDebug() << "All data inserted successfully";
        return true;
    }
    else
    {
        database.rollback();
        qWarning() << "Data insertion failed, transaction rolled back";
        return false;
    }
}

bool DbFacade::updateData(const int id, const QJsonObject &dataObject) const
{
    QJsonDocument doc(dataObject);
    QString jsonDataString = doc.toJson(QJsonDocument::Compact);

    QSqlQuery query(database);
    query.prepare("UPDATE my_data_table SET data = :jsonData WHERE id = :id");
    query.bindValue(":jsonData", jsonDataString);
    query.bindValue(":id", id);

    if (query.exec())
    {
        if (query.numRowsAffected() > 0)
        {
            qDebug() << "Data updated successfully for ID: " << id;
            return true;
        }
        else
        {
            qWarning() << "No rows affected for update on ID: " << id;
            return false;
        }
    }
    else
    {
        qCritical() << "Failed to update data for ID " << id << ":" << query.lastError().text();
        return false;
    }
}

bool DbFacade::deleteData(const int id) const
{
    if (id == -1)
    {
        qWarning() << "Deletion by id is not possible";
        return false;
    }

    QSqlQuery query(database);
    query.prepare("DELETE FROM my_data_table WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec())
    {
        if (query.numRowsAffected() > 0)
        {
            qDebug() << "Data deleted successfully for ID:" << id;
            return true;
        }
        else
        {
            qWarning() << "No rows affected for delete on ID:"
                       << id << " Record might not exist";
            return false;
        }
    }
    else
    {
        qCritical() << "Failed to delete data for ID"
                    << id << ":" << query.lastError().text();
        return false;
    }
}
