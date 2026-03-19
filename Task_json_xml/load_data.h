#ifndef LOAD_DATA_H
#define LOAD_DATA_H


#include <QThread>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QXmlStreamReader>
#include <QDir>
#include <QFileDialog>
#include <QDebug>


class LoadData final : public QThread
{
    Q_OBJECT
public:
    explicit LoadData(QObject *parent = nullptr);
    ~LoadData() = default;

    void setPathFiles(const QStringList& pathList);
    void run() override;

signals:
    void infoData(const QString& str, const QColor& color) const;
    void getRange(const int range) const;
    void getCountRecord(const int count) const;
    void getCountErrorAndGood(const int numError, const int numGood) const;
    void jsonArrayForModel(const QJsonArray& arr) const;

private:
    QStringList listPathFile;
    int countRecord{};
    int countError{};
    int countGood{};

    void countingNumberError(const QString& strError);

    void loadDataFromFile(const QString &filePath);
    bool parseJsonData(const QJsonDocument &jsonDoc);
    QJsonObject convertXmlFileToJsonObject(const QString &filePath);
};

#endif // LOAD_DATA_H
