#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QProcess>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_Save_clicked();
    void on_Run_clicked();
    void on_DeleteFromList_clicked();
    void on_RunFromList_clicked();
    void on_commandFinished(int exitCode, QProcess::ExitStatus exitStatus);  // Slot for command completion
    void on_readyReadStandardOutput();  // Slot for reading output
    void on_readyReadStandardError();  // Slot for reading error output

private:
    void loadCommandList();  // Load saved commands
    void saveCommandList();  // Save commands to persistent storage

    Ui::MainWindow *ui;
    QStringListModel *model;  // Model for the list view
    QStringList stringList;  // Container for the list items
    QProcess *process;  // For running terminal commands
    QSettings *settings;  // For persistent storage
};
#endif // MAINWINDOW_H
