#ifndef WINDOW_H
#define WINDOW_H


#include <QWidget>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileDialog>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextStream>
#include <QThread>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QtXml>
#include <QDebug>
#include "load_data.h"
#include "table_model.h"
#include "db_worker.h"


namespace Ui {
class Window;
}

class Window final : public QWidget
{
    Q_OBJECT

    enum class ClearJsonOrXml : int
    {
        none = 0,
        json = 1,
        xml = 2
    };

public:
    explicit Window(QWidget *parent = nullptr);
    ~Window();

private slots:
    void on_pb_loadData_clicked();
    void print(const QString& text, const QColor& color) const;
    void showTableContextMenu(const QPoint& pos);
    void stringToJsonDocument(const QString& jsonString);
    void deleteRecord();
    void exportRecordToJson();
    void exportRecordToXml();
    void on_pb_clearTable_clicked() const;

private:
    Ui::Window *ui;

    LoadData loadData;
    TableModel* model{};
    QThread* dbThread{};
    DbWorker* dbWorker{};

    int selectedRowIndex{-1};
    int numForNameJsonAndXml{};

    ClearJsonOrXml clearJsonOrXml;
    QJsonDocument jsonDoc;

    bool jsonDocumentToFile(const QJsonDocument& doc, const QString& filePath) const;
    QDomDocument jsonToXmlDocument(const QJsonDocument& jsonDoc) const;
    bool domDocumentToFile(const QDomDocument& doc, const QString& filePath) const;
};

#endif // WINDOW_H
