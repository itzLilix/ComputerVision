#include "mainwindow.h"
#include <QMessageBox>

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QFileDialog>
#include <QMenuBar> 
#include <QActionGroup>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    auto* central = new QWidget(this);
    auto* mainLayout = new QHBoxLayout(central);

    m_imageModel = new ImageModel(this);
	m_imageProcessor = new ImageProcessor(this);
    m_imageView = new ImageView(this);

    mainLayout->addWidget(m_imageView, 1);
    setCentralWidget(central);
	mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* rightBarLayout = new QVBoxLayout();
    mainLayout->addLayout(rightBarLayout);
	mainLayout->setStretchFactor(rightBarLayout, 1);
	mainLayout->setStretchFactor(m_imageView, 3);

	auto* m_adjustmentPanel = new ImageAdjustmentPanel(m_imageModel, m_imageProcessor, this);
    rightBarLayout->addWidget(m_adjustmentPanel);

    m_histogram = new HistogramWidget(this);
    connect(m_imageModel, &ImageModel::newImageLoaded, m_histogram, &HistogramWidget::onImageLoaded);
    connect(m_imageModel, &ImageModel::imageChanged, m_histogram, &HistogramWidget::onImageChanged);
	rightBarLayout->addWidget(m_histogram);
    
    rightBarLayout->addStretch();

    createActions();
    createMenus();
    connectErrorSignals();

    connect(m_imageModel, &ImageModel::newImageLoaded,
        m_imageView, &ImageView::setImage);
    connect(m_imageModel, &ImageModel::imageChanged,
        m_imageView, &ImageView::setImage);
    connect(m_imageModel, &ImageModel::imageClosed, m_imageView,
        [this] { m_imageView->setImage(QImage()); });

    connect(m_imageView, &ImageView::imageDropped, m_imageModel, &ImageModel::loadFile,
        Qt::QueuedConnection);
}

void MainWindow::createActions()
{
    //File actions
    m_openAction = new QAction(tr("&Open..."), this);
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpenClicked);

    m_saveAction = new QAction(tr("&Save..."), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setEnabled(false); // wire up once save logic exists

    m_closeImageAction = new QAction(tr("&Close Image"), this);
    m_closeImageAction->setShortcut(QKeySequence::Close);
    m_closeImageAction->setEnabled(false);
    connect(m_closeImageAction, &QAction::triggered, this, &MainWindow::onCloseClicked);

    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    //Image actions
    m_showMetadataAction = new QAction(tr("&Metadata..."), this);
    connect(m_showMetadataAction, &QAction::triggered, this, &MainWindow::onImageMetadataTriggered);

    m_rotateActionGr = new QActionGroup(this);
    QAction* rotateLeftAction = m_rotateActionGr->addAction(tr("Rotate Left"));
    connect(rotateLeftAction, &QAction::triggered, this, [this]() {
            m_imageModel->rotate(-90);
        });
    QAction* rotateRightAction = m_rotateActionGr->addAction(tr("Rotate Right"));
    connect(rotateRightAction, &QAction::triggered, this, [this]() {
            m_imageModel->rotate(90);
        });
    QAction* rotate180Action = m_rotateActionGr->addAction(tr("Rotate 180"));
    connect(rotate180Action, &QAction::triggered, this, [this]() {
            m_imageModel->rotate(180);
        });

    m_flipActionGr = new QActionGroup(this);
	QAction* flipHorizontalAction = m_flipActionGr->addAction(tr("Flip Horizontal"));
	connect(flipHorizontalAction, &QAction::triggered, this, [this]() {
			m_imageModel->flip(true, false);
		});
	QAction* flipVerticalAction = m_flipActionGr->addAction(tr("Flip Vertical"));
	connect(flipVerticalAction, &QAction::triggered, this, [this]() {
			m_imageModel->flip(false, true);
		});

    // enable/disable Save + Close based on whether an image is loaded
    connect(m_imageModel, &ImageModel::newImageLoaded, this, [this] {
        m_saveAction->setEnabled(true);
        m_closeImageAction->setEnabled(true);
        });
    connect(m_imageModel, &ImageModel::imageClosed, this, [this] {
        m_saveAction->setEnabled(false);
        m_closeImageAction->setEnabled(false);
        });
}

void MainWindow::createMenus()
{
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_closeImageAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    auto* imageMenu = menuBar()->addMenu(tr("&Image"));
    imageMenu->addAction(m_showMetadataAction);
    imageMenu->addSeparator();
	QMenu* rotateMenu = imageMenu->addMenu(tr("&Rotate"));
	rotateMenu->addActions(m_rotateActionGr->actions());
    QMenu* flipMenu = imageMenu->addMenu(tr("&Flip"));
	flipMenu->addActions(m_flipActionGr->actions());
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

    m_imageModel->loadFile(filePath);
}

void MainWindow::onCloseClicked()
{
    m_imageModel->closeImage();
}

void MainWindow::connectErrorSignals() {
	connect(m_imageModel, &ImageModel::loadFailed, this, [this](LoadError error, const QString& filepath) {
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

void MainWindow::onImageMetadataTriggered()
{
    if (!m_imageInfoDialog) {
        m_imageInfoDialog = new ImageInfoDialog(this, m_imageModel);
    }

    m_imageInfoDialog->show();
    m_imageInfoDialog->raise();
    m_imageInfoDialog->activateWindow();
}