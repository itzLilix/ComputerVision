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
    void onUploadClicked();
    void onClearClicked();

private:
    QPushButton* uploadButton;
    QPushButton* clearButton;

    ImageModel* imageModel;
    ImageView* imageView;
};