#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

class Schema;
class SchemaModel;
class SchemaProxyModel;

class SchemaView;
class LineEdit;
class QPlainTextEdit;
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private:
    bool save();
    bool open();
    bool saveAs();
    void close();

private:
    void setupUi();
    void updateWindowTitle();
    void editSchemaProperties();

private:
    struct Ui {
        Ui(MainWindow *mainWindow);
        QWidget *centralWidget;
        SchemaView *view;
        QMenuBar *menuBar;
        QMenu *fileMenu;
        QStatusBar *statusBar;
        QAction *openAction;
        QAction *saveAction;
        QAction *saveAsAction;
        QAction *closeAction;
        QAction *quitAction;
        QMenu *settingsMenu;
        QAction *schemaPropsAction;
        QMenu *generatorMenu;
        QAction *zmkGeneratorAction;
        QAction *qmkGeneratorAction;
        QToolBar *filterBar;
        LineEdit *regexEdit;
        QComboBox *handSelector;
        QDockWidget *noteDockWidget;
        QPlainTextEdit *noteEdit;
    } ui;
    std::unique_ptr<Schema> m_schema;
    SchemaModel *m_model;
    SchemaProxyModel *m_proxyModel;
};
#endif // MAINWINDOW_HPP
