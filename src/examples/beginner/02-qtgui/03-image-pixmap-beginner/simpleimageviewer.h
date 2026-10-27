// QtGui 入门示例 03: 简易图片查看器
#pragma once

#include <QWidget>

#include "imagedisplaywidget.h"

class QLabel;

class SimpleImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleImageViewer(QWidget *parent = nullptr);

private:
    /// @brief 动态生成一个简易文件夹图标（不需要图片文件）
    QIcon createFolderIcon();

    void openFile();

    bool m_fitMode = true;
    QLabel *m_infoLabel = nullptr;
    ImageDisplayWidget *m_imageWidget = nullptr;
};
