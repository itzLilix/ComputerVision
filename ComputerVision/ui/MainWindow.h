#pragma once

#include "../image/ImageModel.h"
#include "ImageView.h"

#include <QMainWindow>
#include <QImage>

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    void setImage(QImage img);

private slots:
    void onOpenClicked();
    void onCloseClicked();

private:
    ImageModel* imageModel;
    
    ImageView* imageView;

    QAction* openAction;
    QAction* saveAction;
    QAction* closeImageAction;
    QAction* exitAction;

    void createActions();
    void createMenus();
	void connectErrorSignals();
};