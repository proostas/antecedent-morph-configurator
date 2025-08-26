#include "mainwindow.hpp"
#include "schema.hpp"
#include "schemamodel.hpp"
#include "schemaview.hpp"
#include <QApplication>
#include <QGridLayout>
#include <QMenuBar>
#include <QStatusBar>
#include "schemapropertiesdialog.hpp"
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QToolBar>
#include "lineedit.hpp"
#include <QRegularExpression>
#include <QDockWidget>
#include <QPlainTextEdit>
#include <QCloseEvent>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui{this},
      m_schema(std::make_unique<Schema>(Schema::Flat)),
      m_model{new SchemaModel{m_schema.get(), this}},
      m_proxyModel{new SchemaProxyModel{this}}
{
    updateWindowTitle();

    connect(ui.saveAction, &QAction::triggered, this, &MainWindow::save);
    connect(ui.saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui.openAction, &QAction::triggered, this, &MainWindow::open);
    connect(ui.closeAction, &QAction::triggered, this, &MainWindow::close);
    connect(ui.quitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(ui.schemaPropsAction, &QAction::triggered, this, &MainWindow::editSchemaProperties);

    m_proxyModel->setSourceModel(m_model);
    connect(ui.regexEdit, &QLineEdit::textChanged, m_proxyModel, qOverload<QString const &>(&SchemaProxyModel::setFilterRegularExpression));
    ui.view->setModel(m_proxyModel);

    ui.view->setColumnWidth(SchemaModel::NameColumn, 150);

    resize(900, 600);

    ui.noteEdit->setEnabled(false);

    connect(ui.view->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [=](QModelIndex const &current, QModelIndex const &previous) {
                int curAntecedentType = current.data(SchemaModel::AntecedentType).toInt();
                int prevAntecedentType = previous.data(SchemaModel::AntecedentType).toInt();

                if (current.isValid() && (curAntecedentType != prevAntecedentType || !previous.isValid()) ) {
                    ui.noteEdit->blockSignals(true);
                    ui.noteEdit->setPlainText(current.data(SchemaModel::Note).toString());
                    ui.noteEdit->blockSignals(false);
                }
                ui.noteEdit->setEnabled(current.isValid());
            });
    connect(ui.noteEdit, &QPlainTextEdit::textChanged, this,
            [this](){
                m_proxyModel->setData(ui.view->selectionModel()->currentIndex(), ui.noteEdit->toPlainText(), SchemaModel::Note);
            });
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_schema->isChanged() && QMessageBox::question(this, "Schema Modified", "Exit without save?",
                QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    {
        event->ignore();
        return;
    }

    event->accept();
}

bool MainWindow::save()
{
    QString filePath;
    if (m_schema->isNew()) {
        filePath = QFileDialog::getSaveFileName(
            this, "Save Configuration",
            QString{"%1/%2"}
                    .arg(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0))
                    .arg(m_schema->suggestedFileName()),
            "All Files (*);;AMConf Files (*.amconf)");
        if (filePath.isEmpty())
            return false;
    } else {
        filePath = m_schema->filePath();
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << m_schema->toJson().toJson();

    if (m_schema->isNew()) {
        m_model->beforeSchemaChange();
        m_schema->setFilePath(filePath);
        m_model->afterSchemaChange();
        updateWindowTitle();
    }

    m_schema->clearChanged();

    return true;
}

bool MainWindow::open()
{
    QString filePath = QFileDialog::getOpenFileName(
                this, "Open Configuration",
                QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0),
                "All Files (*);;AMConf Files (*.amconf)");
    if (filePath.isEmpty())
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return false;

    QJsonParseError error;
    auto json = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        ui.statusBar->showMessage(QString{"Failed to parse document: %1"}.arg(error.errorString()));
        return false;
    }

    bool res = m_schema->fromJson(json);
    if (res) {
        m_schema->setFilePath(filePath);
        m_model->beforeSchemaChange();
        m_model->afterSchemaChange();

        ui.noteEdit->blockSignals(true);
        ui.noteEdit->setPlainText(ui.view->selectionModel()->currentIndex().data(SchemaModel::Note).toString());
        ui.noteEdit->blockSignals(false);
        ui.noteEdit->setEnabled(ui.view->selectionModel()->currentIndex().isValid());

        updateWindowTitle();
    }

    return res;
}

bool MainWindow::saveAs()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "Save Configuration",
        QString{"%1/%2"}
                .arg(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0))
                .arg(m_schema->suggestedFileName()),
        "All Files (*);;AMConf Files (*.amconf)");
    if (filePath.isEmpty())
        return false;

    m_schema->setFilePath(filePath);
    return save();
}

void MainWindow::close()
{
    m_model->beforeSchemaChange();
    m_schema->clear();
    m_model->afterSchemaChange();
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    QString title{"Antecedent Morph Configurator - "};
    if (m_schema->isNew())
        title += "New Schema";
    else
        title += m_schema->fullName();

    setWindowTitle(title);
}

void MainWindow::editSchemaProperties()
{
    SchemaPropertiesDialog dialog{this};
    dialog.setName(m_schema->name());
    dialog.setVersion(m_schema->version());
    dialog.setType(m_schema->type());
    dialog.setPrefix(m_schema->prefix());
    if (dialog.exec() == QDialog::Accepted) {
        m_schema->setName(dialog.name());
        m_schema->setVersion(dialog.version());
        if (m_schema->setType(static_cast<Schema::Type>(dialog.type()))) {
            m_model->beforeSchemaChange();
            m_model->afterSchemaChange();
        }
        m_schema->setPrefix(dialog.prefix());
        updateWindowTitle();
    }
}

MainWindow::Ui::Ui(MainWindow *mainWindow)
    : centralWidget{new QWidget{mainWindow}},
      view{new SchemaView{centralWidget}},
      menuBar{new QMenuBar{mainWindow}},
      fileMenu{new QMenu{menuBar}},
      statusBar{new QStatusBar{mainWindow}},
      openAction{new QAction{mainWindow}},
      saveAction{new QAction{mainWindow}},
      saveAsAction{new QAction{mainWindow}},
      closeAction{new QAction{mainWindow}},
      quitAction{new QAction{mainWindow}},
      settingsMenu{new QMenu{menuBar}},
      schemaPropsAction{new QAction{mainWindow}},
      noteDockWidget{new QDockWidget{"Antecedent Note", mainWindow}},
      noteEdit{new QPlainTextEdit}
{
    mainWindow->setCentralWidget(view);

    menuBar->setGeometry(QRect(0, 0, 800, 22));

    mainWindow->setMenuBar(menuBar);

    mainWindow->setStatusBar(statusBar);

    fileMenu->setTitle("File");
    menuBar->addAction(fileMenu->menuAction());

    openAction->setText("Open schema");
    openAction->setShortcut(QKeySequence{"Ctrl+O"});
    fileMenu->addAction(openAction);

    saveAction->setText("Save schema");
    saveAction->setShortcut(QKeySequence{"Ctrl+S"});
    fileMenu->addAction(saveAction);

    saveAsAction->setText("Save schema as...");
    fileMenu->addAction(saveAsAction);

    closeAction->setText("Close schema");
    fileMenu->addAction(closeAction);

    fileMenu->addSeparator();

    quitAction->setText("Quit");
    quitAction->setShortcut(QKeySequence{"Ctrl+Q"});
    fileMenu->addAction(quitAction);

    settingsMenu->setTitle("Settings");
    menuBar->addAction(settingsMenu->menuAction());

    schemaPropsAction->setText("Schema properties");
    settingsMenu->addAction(schemaPropsAction);

    // ToolBars
    filterBar = mainWindow->addToolBar("Filter");
    regexEdit = new LineEdit;
    regexEdit->setPlaceholderText("Type regex to filter...");
    filterBar->addWidget(regexEdit);

    // Dock Area
    noteDockWidget->setWidget(noteEdit);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, noteDockWidget);
}
