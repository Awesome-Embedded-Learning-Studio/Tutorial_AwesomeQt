/// @file    custom_splitter.cpp
/// @brief   Implementation of CustomSplitter with CustomHandle factory.
///
/// 对应教程：进阶层 03-QtWidgets/42-QSplitter 自定义拖动手柄外观。

#include "custom_splitter.h"

CustomSplitter::CustomSplitter(Qt::Orientation orientation, QWidget* parent)
    : QSplitter(orientation, parent)
{
}

QSplitterHandle* CustomSplitter::createHandle()
{
    return new CustomHandle(orientation(), this);
}
