#include "load_data.h"

LoadData::LoadData(QObject *parent)
    : QThread{parent}
{
}

void LoadData::setPathFiles(const QStringList &pathList)
{
    listPathFile = pathList;
}

void LoadData::countingNumberError(const QString &strError)
{
    ++countError;
    emit infoData(strError + "\n\n", QColor("#ff0000"));
}

void LoadData::run()
{
    emit getRange(listPathFile.size());
    countRecord = 0;
    countError = 0;
    countGood = 0;

    for (const QString& filePath : listPathFile)
    {
        emit infoData("Open file: " + filePath, QColor("#000000"));
        loadDataFromFile(filePath);
    }
    emit getCountErrorAndGood(countError, countGood);
}

void LoadData::loadDataFromFile(const QString &filePath)
{
    QFile file(filePath);

    if (filePath.endsWith(".json", Qt::CaseInsensitive))
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "Could not open file: " << filePath << file.errorString();
            return;
        }

        QByteArray fileContent = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(fileContent, &parseError);

        if (parseError.error != QJsonParseError::NoError)
        {
            qWarning() << "JSON parse error: " << parseError.errorString();
            countingNumberError("JSON parse error: " + parseError.errorString());
        }
        else
        {
            if (!parseJsonData(jsonDoc))
            {
                qWarning() << "Error when processing JSON data";
                countingNumberError("Error when processing JSON data!");
            }
        }

    }
    else if (filePath.endsWith(".xml", Qt::CaseInsensitive))
    {
        if (!parseJsonData(QJsonDocument(convertXmlFileToJsonObject(filePath))))
        {
            qWarning() << "Error when processing XML data";
            countingNumberError("Error when processing XML data!");
            return;
        }
    }
    else
    {
        qWarning() << "Unknown file format: " << filePath;
        countingNumberError("Unknown file format: " + filePath);
    }
}

bool LoadData::parseJsonData(const QJsonDocument &jsonDoc)
{
    if (jsonDoc.isNull())
    {
        qWarning() << "JSON document is null";
        countingNumberError("JSON document is null");
        return false;
    }

    QStringList headers;
    QJsonArray dataArrayForModel;

    if (jsonDoc.isObject())
    {
        QJsonObject rootObj = jsonDoc.object();

        if (rootObj.contains("root") && rootObj.value("root").isObject())
        {
            QJsonObject nestedDataObject = rootObj.value("root").toObject();
            headers = nestedDataObject.keys();
            dataArrayForModel.append(nestedDataObject);
        }
    }

    if (!headers.isEmpty() && !dataArrayForModel.isEmpty())
    {
        emit getCountRecord(++countRecord);
        emit infoData("The data is read \n\n", QColor("#5fe915"));
        emit jsonArrayForModel(dataArrayForModel);
        ++countGood;
    }
    return true;
}

QJsonObject LoadData::convertXmlFileToJsonObject(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Could not open XML file for reading: "
                   << filePath << file.errorString();
        emit infoData("JCould not open XML file for reading: " + file.errorString(),
                      QColor("#ff0000"));
        return QJsonObject();
    }

    QXmlStreamReader reader(&file);
    QJsonObject rootObject;
    QJsonObject dataObject;
    QStringList headers;
    bool inRoot = false;

    while (!reader.atEnd() && !reader.hasError())
    {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::StartElement)
        {
            QString elementName = reader.name().toString();
            if (elementName == "root")
            {
                inRoot = true;
                continue;
            }

            if (inRoot)
            {
                QString elementValue;
                if (reader.readNext() == QXmlStreamReader::Characters)
                {
                    elementValue = reader.text().toString();
                    reader.readNext();
                }
                if (elementValue.toLower() == "true")
                {
                    dataObject.insert(elementName, true);
                }
                else if (elementValue.toLower() == "false")
                {
                    dataObject.insert(elementName, false);
                }
                else
                {
                    bool okDouble;
                    double doubleValue = elementValue.toDouble(&okDouble);
                    if (okDouble)
                    {
                       dataObject.insert(elementName, doubleValue);
                    }
                    else
                    {
                       dataObject.insert(elementName, elementValue);
                    }
                }
                headers.append(elementName);
            }
        }
        else if (token == QXmlStreamReader::EndElement && reader.name().toString() == "root")
        {
            inRoot = false;
        }
    }

    if (reader.hasError())
    {
        qWarning() << "XML stream error: " << reader.errorString();
        countingNumberError("XML stream error: " + reader.errorString());
        return QJsonObject();
    }
    file.close();

    rootObject.insert("root", dataObject);
    return rootObject;
}
