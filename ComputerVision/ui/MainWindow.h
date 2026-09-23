#pragma once

#include "../image/ImageModel.h"
#include "ImageView.h"
#include "ImageInfo.h"
#include "ImageAdjustmentPanel.h"
#include "HistogramWidget.h"

#include <QMainWindow>
#include <QPointer>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onOpenClicked();
    void onCloseClicked();
    void onImageMetadataTriggered();

private:
    ImageModel* m_imageModel;
    ImageProcessor* m_imageProcessor;
    
    ImageView* m_imageView;
    HistogramWidget* m_histogram;
    ImageAdjustmentPanel* m_adjustmentPanel;
    
    QPointer<ImageInfoDialog> m_imageInfoDialog;

    //File actions
    QAction* m_openAction;
    QAction* m_saveAction;
    QAction* m_closeImageAction;
    QAction* m_exitAction;

    //Image actions
    QAction* m_showMetadataAction;
    QActionGroup* m_rotateActionGr;
    QActionGroup* m_flipActionGr;

    void createActions();
    void createMenus();
	void connectErrorSignals();
};