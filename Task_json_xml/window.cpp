#include "window.h"
#include "ui_window.h"

Window::Window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);

    model = new TableModel{this};
    ui->table->setModel(model);
    ui->table->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->table, &QTableView::customContextMenuRequested,
            this, &Window::showTableContextMenu);

    ui->progressLoadData->reset();

    dbThread = new QThread(this);
    dbWorker = new DbWorker("./database.db");
    dbWorker->moveToThread(dbThread);
    dbThread->start();

    connect(dbThread, &QThread::finished, dbWorker, &QObject::deleteLater);
    connect(dbThread, &QThread::finished, dbThread, &QThread::deleteLater);

    connect(&loadData, &LoadData::jsonArrayForModel, this, [this](const QJsonArray& arr)
    {
        dbWorker->insertData(arr);
    }, Qt::QueuedConnection);

    connect(dbWorker, &DbWorker::updateModel, model, &TableModel::updateModelData,
            Qt::QueuedConnection);

    connect(ui->pb_loadDb, &QPushButton::clicked, this, [this]()
    {
        dbWorker->loadAllData();
    }, Qt::QueuedConnection);

    connect(dbWorker, &DbWorker::stringToJsonDocument, this, &Window::stringToJsonDocument,
            Qt::QueuedConnection);

    connect(&loadData, &LoadData::getRange, this, [this](const int range)
    {
        ui->progressLoadData->reset();
        ui->progressLoadData->setRange(0, range);
    }, Qt::BlockingQueuedConnection);

    connect(&loadData, &LoadData::getCountRecord, this, [this](const int count)
    {
        ui->progressLoadData->setValue(count);
    }, Qt::BlockingQueuedConnection);

    connect(&loadData, &LoadData::infoData, this,
            &Window::print, Qt::BlockingQueuedConnection);

    connect(&loadData, &LoadData::getCountErrorAndGood, this,
            [this](const int numError, const int numGood)
    {
        print("Number of Error: " + QString::number(numError), QColor("#ff0000"));
        print("Number of operations performed: " + QString::number(numGood), QColor("#5fe915"));
    }, Qt::BlockingQueuedConnection);

    connect(model, &TableModel::updateData, dbWorker, &DbWorker::updateData, Qt::QueuedConnection);
}

Window::~Window()
{
    delete ui;
}

void Window::on_pb_loadData_clicked()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this, "Open file",
                                                    "../TestFolder",
                                                    "All (*);;file (*.xml *.json)");
    if (filePaths.size() > 0)
    {
        loadData.setPathFiles(filePaths);
        loadData.start();
    }
}

void Window::print(const QString &text, const QColor &color) const
{
    QTextCursor cursor = ui->infoData->textCursor();
    QTextCharFormat format;

    format.setForeground(color);
    cursor.setCharFormat(format);
    cursor.insertText(text + "\n");
}

void Window::showTableContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);
    QModelIndex index = ui->table->indexAt(pos);

    if (!index.isValid())
    {
        return;
    }

    QAction deleteAction("Удалить", this);
    QAction exportJsonAction("Экспортировать в JSON", this);
    QAction exportXmlAction("Экспортировать в XML", this);

    connect(&deleteAction, &QAction::triggered, this, &Window::deleteRecord);
    connect(&exportJsonAction, &QAction::triggered, this, &Window::exportRecordToJson);
    connect(&exportXmlAction, &QAction::triggered, this, &Window::exportRecordToXml);

    contextMenu.addAction(&deleteAction);
    contextMenu.addSeparator();
    contextMenu.addAction(&exportJsonAction);
    contextMenu.addAction(&exportXmlAction);

    QPoint point;
    point.setX(0);
    point.setY(pos.y());

    selectedRowIndex = ui->table->indexAt(point).data().toInt();

    contextMenu.exec(ui->table->viewport()->mapToGlobal(pos));
}

void Window::stringToJsonDocument(const QString &jsonString)
{
    QJsonParseError parseError;
    jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qCritical() << "Error when parsing json: " << parseError.errorString();
    }
    else
    {
        if (clearJsonOrXml == ClearJsonOrXml::json)
        {
            QString nameFileJson = "output" + QString::number(numForNameJsonAndXml++) + ".json";

            if (jsonDocumentToFile(jsonDoc, nameFileJson))
            {
                QMessageBox::information(this, "Information",
                                         "The  \"" + nameFileJson + "\"  file has been created",
                                         QMessageBox::Close, QMessageBox::Close);
            }
            else
            {
                QMessageBox::critical(this, "Warning",
                                      "Error ocuurred when creating the file",
                                      QMessageBox::Ok, QMessageBox::Ok);
            }
        }
        else if (clearJsonOrXml == ClearJsonOrXml::xml)
        {
            QDomDocument xmlDoc = jsonToXmlDocument(jsonDoc);

            if (!xmlDoc.isNull())
            {
                QString nameFileXml = "output" + QString::number(numForNameJsonAndXml++) + ".xml";

                if (domDocumentToFile(xmlDoc, nameFileXml))
                {
                    QMessageBox::information(this, "Information",
                                             "The  \"" + nameFileXml + "\"  file has been created",
                                             QMessageBox::Close, QMessageBox::Close);
                }
                else
                {
                    QMessageBox::critical(this, "Warning",
                                          "Error ocuurred when creating the file",
                                          QMessageBox::Ok, QMessageBox::Ok);
                }
            }
            else
            {
                qCritical() << "Error create xml document";
            }
        }
        else
        {
            qWarning() << "Error! undefined type";
        }
    }
    clearJsonOrXml = ClearJsonOrXml::none;
}

void Window::deleteRecord()
{
    if (selectedRowIndex == -1)
    {
        return;
    }

    if (QMessageBox::question(this, "Deleting an entry",
                              "Are you sure you want to delete the record?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    {
        return;
    }
    dbWorker->deleteData(selectedRowIndex);
    selectedRowIndex = -1;
}

void Window::exportRecordToJson()
{
    if (selectedRowIndex == -1)
    {
        return;
    }
    dbWorker->loadData(selectedRowIndex);
    clearJsonOrXml = ClearJsonOrXml::json;
    selectedRowIndex = -1;
}

void Window::exportRecordToXml()
{
    if (selectedRowIndex == -1)
    {
        return;
    }
    dbWorker->loadData(selectedRowIndex);
    clearJsonOrXml = ClearJsonOrXml::xml;
    selectedRowIndex = -1;
}

bool Window::jsonDocumentToFile(const QJsonDocument &doc, const QString &filePath) const
{
    QFile outFile(filePath);

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical() << "Couldn open the file for writing json: "
                    << filePath << outFile.errorString();
        return false;
    }

    QTextStream out(&outFile);
    out << doc.toJson(QJsonDocument::Indented);

    outFile.close();
    return true;
}

QDomDocument Window::jsonToXmlDocument(const QJsonDocument &jsonDoc) const
{
    QDomDocument domDoc("root");

    if (jsonDoc.isNull() || !jsonDoc.isObject())
    {
        qCritical() << "Invalid json document";
        return QDomDocument();
    }

    QJsonObject jsonObj = jsonDoc.object();
    QDomElement rootElement = domDoc.createElement("root");
    domDoc.appendChild(rootElement);


    std::function<void(QDomElement, const QJsonObject&)> addElements =
        [&](QDomElement parentElement, const QJsonObject& obj)
    {
        for (auto it = obj.begin(); it != obj.end(); ++it)
        {
            QDomElement newElement = domDoc.createElement(it.key());
            QJsonValue value = it.value();

            if (value.isObject())
            {
                parentElement.appendChild(newElement);
                addElements(newElement, value.toObject());
            }
            else if (value.isArray())
            {
                QDomElement arrayWrapper = domDoc.createElement(it.key() + "_list");
                parentElement.appendChild(arrayWrapper);
                const QJsonArray arr = value.toArray();

                for (const auto& arrVal : arr)
                {
                    QDomElement itemElement = domDoc.createElement("item");
                    itemElement.appendChild(domDoc.createTextNode(arrVal.toVariant().toString()));
                    arrayWrapper.appendChild(itemElement);
                }
            }
            else
            {
                QDomText textNode = domDoc.createTextNode(value.toVariant().toString());
                newElement.appendChild(textNode);
                parentElement.appendChild(newElement);
            }
        }
    };
    addElements(rootElement, jsonObj);
    return domDoc;
}

bool Window::domDocumentToFile(const QDomDocument &doc, const QString &filePath) const
{
    QFile outFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical() << "Couldn open the file for writing XML: "
                    << filePath << outFile.errorString();
        return false;
    }

    QTextStream out(&outFile);
    out << doc.toString(4);

    outFile.close();
    return true;
}

void Window::on_pb_clearTable_clicked() const
{
    model->clearModel();
}
