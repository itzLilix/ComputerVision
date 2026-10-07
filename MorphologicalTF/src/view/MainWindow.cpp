#include "view/MainWindow.h"

#include "view/ImageView.h"
#include "view/KernelPanel.h"
#include "view/ProcessingPanel.h"
#include "viewmodel/ImageViewModel.h"
#include "viewmodel/KernelViewModel.h"
#include "viewmodel/ProcessingViewModel.h"

#include <QAction>
#include <QCloseEvent>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QStatusBar>
#include <QUrl>

namespace {

// Returns the first dropped local file path, or an empty string
QString firstLocalFile(const QMimeData *mime)
{
    if (!mime->hasUrls())
        return {};
    const QList<QUrl> urls = mime->urls();
    if (urls.isEmpty() || !urls.first().isLocalFile())
        return {};
    return urls.first().toLocalFile();
}

} // namespace

MainWindow::MainWindow(ImageViewModel &imageViewModel, KernelViewModel &kernelViewModel,
                       ProcessingViewModel &processingViewModel, QWidget *parent)
    : QMainWindow(parent)
    , m_viewModel(imageViewModel)
    , m_kernelViewModel(kernelViewModel)
    , m_processingViewModel(processingViewModel)
{
    setWindowTitle(tr("Morphology Lab"));
    resize(1300, 800);
    setAcceptDrops(true); // child widgets do not accept drops, so events reach this window

    m_imageView = new ImageView(this);
    setCentralWidget(m_imageView);

    m_statusLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_statusLabel);

    createMenus();
    createDocks();

    // Bindings: view model -> widgets
    connect(&m_viewModel, &ImageViewModel::imageChanged, this, [this] {
        m_imageView->setImage(m_viewModel.currentImage());
        updateUi();
    });
    connect(&m_viewModel, &ImageViewModel::imageCleared, this, [this] {
        m_imageView->clearImage();
        updateUi();
    });
    connect(&m_viewModel, &ImageViewModel::errorOccurred, this, &MainWindow::showError);
    connect(&m_processingViewModel, &ProcessingViewModel::errorOccurred, this, &MainWindow::showError);

    updateUi();
}

void MainWindow::createMenus()
{
    m_openAction = new QAction(tr("&Open..."), this);
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpen);

    m_closeAction = new QAction(tr("&Close Image"), this);
    m_closeAction->setShortcut(QKeySequence::Close);
    connect(m_closeAction, &QAction::triggered, this, &MainWindow::onCloseImage);

    m_saveAction = new QAction(tr("&Save..."), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::onSave);

    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close); // goes through closeEvent

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_closeAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    m_resetAction = new QAction(tr("&Reset to Original"), this);
    m_resetAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(m_resetAction, &QAction::triggered, &m_viewModel, &ImageViewModel::resetToOriginal);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_resetAction);
}

void MainWindow::createDocks()
{
    auto *kernelDock = new QDockWidget(tr("Structuring Element"), this);
    kernelDock->setWidget(new KernelPanel(m_kernelViewModel, kernelDock));
    addDockWidget(Qt::RightDockWidgetArea, kernelDock);

    auto *processingDock = new QDockWidget(tr("Processing"), this);
    processingDock->setWidget(new ProcessingPanel(m_processingViewModel, processingDock));
    addDockWidget(Qt::LeftDockWidgetArea, processingDock);

    // Lets the user bring the dock back after closing it
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(processingDock->toggleViewAction());
    viewMenu->addAction(kernelDock->toggleViewAction());
}

void MainWindow::updateUi()
{
    const bool hasImage = m_viewModel.hasImage();
    m_closeAction->setEnabled(hasImage);
    m_saveAction->setEnabled(hasImage);
    m_resetAction->setEnabled(hasImage);
    m_statusLabel->setText(m_viewModel.statusText());
}

void MainWindow::onOpen()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open Image"), QString(), ImageViewModel::openFileFilter());
    if (!path.isEmpty())
        openPath(path);
}

void MainWindow::onSave()
{
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Save Image"), m_viewModel.suggestedSaveName(), ImageViewModel::saveFileFilter());
    if (!path.isEmpty())
        m_viewModel.saveImage(path);
}

void MainWindow::onCloseImage()
{
    if (confirmDiscardChanges())
        m_viewModel.closeImage();
}

void MainWindow::openPath(const QString &path)
{
    if (confirmDiscardChanges())
        m_viewModel.openImage(path);
}

bool MainWindow::confirmDiscardChanges()
{
    if (!m_viewModel.isModified())
        return true;

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("Unsaved changes"),
        tr("The image has unsaved changes. Save them first?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    if (answer == QMessageBox::Save) {
        onSave();
        return !m_viewModel.isModified(); // still modified if the save dialog was cancelled
    }
    return answer == QMessageBox::Discard;
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::warning(this, tr("Error"), message);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (ImageViewModel::isSupportedImageFile(firstLocalFile(event->mimeData())))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QString path = firstLocalFile(event->mimeData());
    if (!ImageViewModel::isSupportedImageFile(path))
        return;

    event->acceptProposedAction();

    // Defer the work: a modal dialog opened inside dropEvent
    // would freeze the drag source (e.g. Windows Explorer).
    QMetaObject::invokeMethod(this, [this, path] { openPath(path); }, Qt::QueuedConnection);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (confirmDiscardChanges())
        event->accept();
    else
        event->ignore();
}
