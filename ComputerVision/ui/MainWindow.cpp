#include "mainwindow.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    uploadButton = new QPushButton(tr("Upload image"), this);
    layout->addWidget(uploadButton);

    resetButton = new QPushButton(tr("Reset image"), this);
    layout->addWidget(resetButton);

    setCentralWidget(central);

    connect(uploadButton, &QPushButton::clicked,
        this, &MainWindow::onUploadClicked);
    connect(resetButton, &QPushButton::clicked,
        this, &MainWindow::onResetClicked);
}

void MainWindow::onUploadClicked()
{
    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Images (*.jpg *.jpeg *.png *.bmp);;All Files (*)");
    
    if (dialog.exec()) {
        QStringList selectedFiles = dialog.selectedFiles();
        if (selectedFiles.isEmpty()) {
            return;
        }
        
        const QString filePath = selectedFiles.constFirst();
        qDebug() << "Selected file:" << filePath;
        
        QImage img(filePath);
        if (img.isNull()) {
            qDebug() << "Failed to load image:" << filePath;
            return;
        }

        setImage(std::move(img));
    }
}

void MainWindow::onResetClicked()
{
    setImage(QImage());
}

void MainWindow::setImage(QImage img)
{
    uploadedImage.swap(img);
    qDebug() << uploadedImage.size();
}

