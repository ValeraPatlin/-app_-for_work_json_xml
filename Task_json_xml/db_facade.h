#ifndef DB_FACADE_H
#define DB_FACADE_H

#include <QObject>
#include <QtSql>
#include <QSqlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QDebug>
#include "table_model.h"


class DbFacade final : public QObject
{
    Q_OBJECT
public:
    explicit DbFacade(const QString& dbName, QObject *parent = nullptr);
    ~DbFacade();

    QJsonArray loadAllData() const;
    QString loadData(const int id) const;
    bool insertData(const QJsonArray& dataArray);
    bool updateData(const int id, const QJsonObject& dataObject) const;
    bool deleteData(const int id) const;

private:
    QSqlDatabase database;
    QString nameDb;

    bool openDatabase();
    void closeDatabase();
};

#endif // DB_FACADE_H
