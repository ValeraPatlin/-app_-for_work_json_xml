#ifndef DB_WORKER_H
#define DB_WORKER_H


#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include "db_facade.h"


class DbWorker final : public QObject
{
    Q_OBJECT
public:
    explicit DbWorker(const QString& dbName, QObject *parent = nullptr);
    ~DbWorker();

    void loadAllData() const;
    void insertData(const QJsonArray& data) const;
    void deleteData(const int id) const;

signals:
    void updateModel(const QJsonArray& data) const;
    void stringToJsonDocument(const QString& strData) const;

public slots:
    void loadData(const int id) const;
    void updateData(const int id, const QJsonObject& dataObject) const;

private:
    DbFacade* dbFacade;
};

#endif // DB_WORKER_H
