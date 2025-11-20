// File for main window actions
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QModelIndex>
#include <QTextCursor>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , model(new QStringListModel(this))
    , process(new QProcess(this))
    , settings(new QSettings("ScriptShelf", "scripts", this))
{
    ui->setupUi(this);

    // Load saved commands before initializing the model
    loadCommandList();

    // Initialize the model with empty list
    model->setStringList(stringList);
    ui->CommandList->setModel(model);

    // Connect the buttons to slots
    connect(ui->Save, &QPushButton::clicked, this, &MainWindow::on_Save_clicked);
    connect(ui->DeleteFromList, &QPushButton::clicked, this, &MainWindow::on_DeleteFromList_clicked);
    connect(ui->RunFromList, &QPushButton::clicked, this, &MainWindow::on_RunFromList_clicked);
    connect(ui->Run, &QPushButton::clicked, this, &MainWindow::on_Run_clicked);

    // Connect process signals
    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::on_readyReadStandardOutput);
    connect(process, &QProcess::readyReadStandardError, this, &MainWindow::on_readyReadStandardError);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::on_commandFinished);

    // Setup the text box for output (use QTextEdit for better display)
    ui->textEdit->setReadOnly(true);
    ui->textEdit->setPlaceholderText("Command output will appear here...");
}

MainWindow::~MainWindow()
{
    // Save commands when application closes
    saveCommandList();
    delete ui;
}

void MainWindow::loadCommandList()
{
    // Load commands from QSettings
    stringList = settings->value("commandList").toStringList();
    qDebug() << "Loaded" << stringList.size() << "commands from storage";
}

void MainWindow::saveCommandList()
{
    // Save commands to QSettings
    settings->setValue("commandList", stringList);
    settings->sync();  // Ensure data is written
    qDebug() << "Saved" << stringList.size() << "commands to storage";
}

void MainWindow::on_Save_clicked()
{
    // Get text from QLineEdit
    QString text = ui->lineEdit->text().trimmed();

    // Only add if text is not empty
    if (!text.isEmpty()) {
        // Add to string list
        stringList.append(text);

        // Update the model
        model->setStringList(stringList);

        // Clear the input field
        ui->lineEdit->clear();

        // Save immediately after adding
        saveCommandList();

        // Optional: Scroll to the bottom to show new item
        QModelIndex index = model->index(stringList.size() - 1);
        ui->CommandList->scrollTo(index);
    }
}

void MainWindow::on_Run_clicked()
{
    // Check if process is already running
    if (process->state() == QProcess::Running){
        QMessageBox::information(this, "Process information", "Process is already running. Please wait!");
        return;
    }

    //
    // Get the command from the input
    QString text = ui->lineEdit->text().trimmed();

    // Work with it if it is not empty
    if (!text.isEmpty()){
        // Clear previous output
        ui->textEdit->clear();

        // Add starting message
        ui->textEdit->append(">>> Running command: " + text + "\n");

        // Disable the run button while command is executing
        ui->RunFromList->setEnabled(false);
        ui->RunFromList->setText("Running...");

        // Start the process
        process->start("bash", QStringList() << "-c" << text);
    }
}

void MainWindow::on_DeleteFromList_clicked()
{
    // Get the currently selected item
    QModelIndex currentIndex = ui->CommandList->currentIndex();

    // Check if an item is selected
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "No Selection", "Please select an item to delete.");
        return;
    }

    // Get the text of the selected item for the confirmation message
    QString selectedText = currentIndex.data(Qt::DisplayRole).toString();

    // Create confirmation dialog
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Delete",
                                  "Are you sure you want to delete:\n\"" + selectedText + "\"?",
                                  QMessageBox::Yes | QMessageBox::No);

    // If user confirms deletion
    if (reply == QMessageBox::Yes) {
        // Remove from string list
        stringList.removeAt(currentIndex.row());

        // Update the model
        model->setStringList(stringList);

        // Save immediately after deleting
        saveCommandList();

        // Optional: Select the next item or clear selection if list is empty
        if (!stringList.isEmpty()) {
            int newIndex = qMin(currentIndex.row(), stringList.size() - 1);
            ui->CommandList->setCurrentIndex(model->index(newIndex));
        }
    }
}

void MainWindow::on_RunFromList_clicked()
{
    // Check if a process is already running
    if (process->state() == QProcess::Running) {
        QMessageBox::information(this, "Process Running",
                                 "A command is already running. Please wait for it to finish.");
        return;
    }

    // Get the currently selected item
    QModelIndex currentIndex = ui->CommandList->currentIndex();

    // Check if an item is selected
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "No Selection", "Please select an item to delete.");
        return;
    }

    // Get the text of the selected item for the confirmation message
    QString command = currentIndex.data(Qt::DisplayRole).toString();

    // Clear previous output
    ui->textEdit->clear();

    // Add starting message
    ui->textEdit->append(">>> Running command: " + command + "\n");

    // Disable the run button while command is executing
    ui->RunFromList->setEnabled(false);
    ui->RunFromList->setText("Running...");

    // Start the process
    process->start("bash", QStringList() << "-c" << command);
}

// Slot for reading standard output
void MainWindow::on_readyReadStandardOutput()
{
    QByteArray output = process->readAllStandardOutput();
    QString text = QString::fromLocal8Bit(output);
    ui->textEdit->setTextColor(Qt::white);
    ui->textEdit->insertPlainText(text);

    // Auto-scroll to bottom
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(cursor);
}

// Slot for reading error output
void MainWindow::on_readyReadStandardError()
{
    QByteArray error = process->readAllStandardError();
    QString text = QString::fromLocal8Bit(error);

    // Display errors in red (optional)
    ui->textEdit->setTextColor(Qt::red);
    ui->textEdit->insertPlainText(text);
    ui->textEdit->setTextColor(Qt::white); // Reset to white

    // Auto-scroll to bottom
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(cursor);
}

// Slot for when command finishes
void MainWindow::on_commandFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Re-enable the run button
    ui->RunFromList->setEnabled(true);
    ui->RunFromList->setText("Run Command");

    // Show completion status
    ui->textEdit->append("\n>>> Command finished with exit code: " + QString::number(exitCode));

    ui->textEdit->setTextColor(Qt::white);
    ui->textEdit->append(">>> Process completed.");

}
