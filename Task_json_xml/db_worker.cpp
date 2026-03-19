#include "db_worker.h"

DbWorker::DbWorker(const QString &dbName, QObject *parent)
    : QObject{parent}
{
    dbFacade = new DbFacade{dbName};
}

DbWorker::~DbWorker()
{
    delete dbFacade;
}

void DbWorker::loadAllData() const
{
    emit updateModel(dbFacade->loadAllData());
}

void DbWorker::insertData(const QJsonArray &data) const
{
    if (dbFacade->insertData(data))
    {
        emit updateModel(dbFacade->loadAllData());
    }
}

void DbWorker::deleteData(const int id) const
{
    if (dbFacade->deleteData(id))
    {
        emit updateModel(dbFacade->loadAllData());
    }
}

void DbWorker::loadData(const int id) const
{
    emit stringToJsonDocument(dbFacade->loadData(id));
}

void DbWorker::updateData(const int id, const QJsonObject &dataObject) const
{
    if (dbFacade->updateData(id, dataObject))
    {
        emit updateModel(dbFacade->loadAllData());
    }
}
