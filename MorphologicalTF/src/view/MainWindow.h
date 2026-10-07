#pragma once

#include <QMainWindow>

class ImageView;
class ImageViewModel;
class KernelViewModel;
class ProcessingViewModel;
class QAction;
class QLabel;

// The View: builds the menu, shows dialogs, accepts drag'n'drop and
// binds the widgets to the view model. All logic lives in ImageViewModel.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ImageViewModel &imageViewModel, KernelViewModel &kernelViewModel,
               ProcessingViewModel &processingViewModel, QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onOpen();
    void onSave();
    void onCloseImage();
    void showError(const QString &message);

private:
    void createMenus();
    void createDocks();
    void openPath(const QString &path);
    bool confirmDiscardChanges();
    void updateUi();

    ImageViewModel &m_viewModel;
    KernelViewModel &m_kernelViewModel;
    ProcessingViewModel &m_processingViewModel;
    ImageView *m_imageView = nullptr;
    QLabel *m_statusLabel = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_closeAction = nullptr;
    QAction *m_saveAction = nullptr;
    QAction *m_exitAction = nullptr;
    QAction *m_resetAction = nullptr;
};
