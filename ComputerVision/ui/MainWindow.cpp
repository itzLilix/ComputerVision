#include "mainwindow.h"
#include <QMessageBox>

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QFileDialog>
#include <QMenuBar> 

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    auto* central = new QWidget(this);
    auto* mainLayout = new QHBoxLayout(central);

    imageModel = new ImageModel(this);
    imageView = new ImageView(this);

    mainLayout->addWidget(imageView, 1);
    setCentralWidget(central);
	mainLayout->setContentsMargins(0, 0, 0, 0);

    // right-side panel stays — this is where ImageInfo/histogram
    // widgets will live once you build them
    auto* rightBarLayout = new QVBoxLayout();
    mainLayout->addLayout(rightBarLayout);
    rightBarLayout->addStretch();

    createActions();
    createMenus();

    connect(imageModel, &ImageModel::newImageLoaded,
        imageView, &ImageView::setImage);
    connect(imageModel, &ImageModel::imageChanged,
        imageView, &ImageView::setImage);
    connect(imageModel, &ImageModel::imageClosed, imageView,
        [this] { imageView->setImage(QImage()); });

    connect(imageView, &ImageView::imageDropped, imageModel, &ImageModel::loadFile,
        Qt::QueuedConnection);
}

void MainWindow::createActions()
{
    openAction = new QAction(tr("&Open..."), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenClicked);

    saveAction = new QAction(tr("&Save..."), this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setEnabled(false); // wire up once save logic exists

    closeImageAction = new QAction(tr("&Close Image"), this);
    closeImageAction->setShortcut(QKeySequence::Close);
    closeImageAction->setEnabled(false);
    connect(closeImageAction, &QAction::triggered, this, &MainWindow::onCloseClicked);

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // enable/disable Save + Close based on whether an image is loaded
    connect(imageModel, &ImageModel::newImageLoaded, this, [this] {
        saveAction->setEnabled(true);
        closeImageAction->setEnabled(true);
        });
    connect(imageModel, &ImageModel::imageClosed, this, [this] {
        saveAction->setEnabled(false);
        closeImageAction->setEnabled(false);
        });

	connectErrorSignals();
}

void MainWindow::createMenus()
{
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(closeImageAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    menuBar()->addMenu(tr("&Image"));  // rotate/grayscale/etc. actions go here later
    menuBar()->addMenu(tr("&View"));   // panel toggles go here later
}

void MainWindow::onOpenClicked()
{
    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Images (*.jpg *.jpeg *.png *.bmp);;All Files (*)");
    
    if (!dialog.exec()) return;

    QStringList selectedFiles = dialog.selectedFiles();
    if (selectedFiles.isEmpty()) {
        return;
    }
        
    const QString filePath = selectedFiles.constFirst();
    qDebug() << "Selected file:" << filePath;

    imageModel->loadFile(filePath);
}

void MainWindow::onCloseClicked()
{
    imageModel->clearImage();
}

void MainWindow::connectErrorSignals() {
	connect(imageModel, &ImageModel::LoadFailed, this, [this](LoadError error, const QString& filepath) {
		QString errorMessage;
		switch (error) {
		case LoadError::FileNotFound:
			errorMessage = tr("File not found: %1").arg(filepath);
			break;
		case LoadError::InvalidFormat:
			errorMessage = tr("Invalid image format: %1").arg(filepath);
			break;
		default:
			errorMessage = tr("Unknown error occurred while loading: %1").arg(filepath);
			break;
		}
		QMessageBox::warning(this, tr("Error"), errorMessage);
		});
}