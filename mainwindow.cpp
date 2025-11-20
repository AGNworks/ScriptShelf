#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QModelIndex>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , model(new QStringListModel(this))
{
    ui->setupUi(this);

    // Initialize the model with empty list
    model->setStringList(stringList);
    ui->CommandList->setModel(model);

    // Connect the buttons to slots
    connect(ui->Save, &QPushButton::clicked, this, &MainWindow::on_Save_clicked);
    connect(ui->DeleteFromList, &QPushButton::clicked, this, &MainWindow::on_DeleteFromList_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
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

        // Optional: Scroll to the bottom to show new item
        QModelIndex index = model->index(stringList.size() - 1);
        ui->CommandList->scrollTo(index);
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

        // Optional: Select the next item or clear selection if list is empty
        if (!stringList.isEmpty()) {
            int newIndex = qMin(currentIndex.row(), stringList.size() - 1);
            ui->CommandList->setCurrentIndex(model->index(newIndex));
        }
    }
}
