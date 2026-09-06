#include "mainwindow.h"
#include <QMessageBox>

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    auto* central = new QWidget(this);
    auto* mainLayout = new QHBoxLayout(central);

    imageModel = new ImageModel(this);
    imageView = new ImageView(this);

    mainLayout->addWidget(imageView, 1);

    setCentralWidget(central);

    auto* rightBarLayout = new QVBoxLayout();
    auto* setImgBtnsLayout = new QHBoxLayout();

    uploadButton = new QPushButton(tr("Upload image"), this);
    clearButton = new QPushButton(tr("Clear image"), this);

    setImgBtnsLayout->addWidget(uploadButton);
    setImgBtnsLayout->addWidget(clearButton);
    
    rightBarLayout->addLayout(setImgBtnsLayout);
    mainLayout->addLayout(rightBarLayout);

    rightBarLayout->addStretch();



    connect(uploadButton, &QPushButton::clicked,
        this, &MainWindow::onUploadClicked);
    connect(clearButton, &QPushButton::clicked,
        this, &MainWindow::onClearClicked);

    connect(imageModel, &ImageModel::newImageLoaded,
        imageView, &ImageView::setImage);
    connect(imageModel, &ImageModel::imageChanged,
        imageView, &ImageView::setImage);
    connect(imageModel, &ImageModel::imageCleared, imageView,
        [imageView = this->imageView] { imageView->setImage(QImage()); });

    connect(imageView, &ImageView::imageDropped, imageModel, &ImageModel::loadFile);
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

        if (!imageModel->loadFile(filePath)) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to load image."));
        }
    }
}

void MainWindow::onClearClicked()
{
    imageModel->clearImage();
}