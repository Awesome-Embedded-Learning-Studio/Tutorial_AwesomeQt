/// @file    encoding_demo.h
/// @brief   演示 QString 编码转换与隐式构造开销。
///
/// 对应教程：进阶层 01-QtBase/03-QString 进阶。
/// 演示 fromUtf8 / fromLatin1 / fromUtf16 的区别、隐式转换的 QByteArray
/// 中间对象开销、以及 Latin-1 vs UTF-8 处理非 ASCII 字符的陷阱。

#pragma once

class EncodingDemo
{
public:
    /// @brief 运行全部编码转换演示。
    static void runAll();

    /// @brief 演示 fromUtf8 / fromLatin1 / fromUtf16 的使用和区别。
    static void demoEncodingConversions();

    /// @brief 演示隐式 const char* -> QString 转换产生的临时对象开销。
    static void demoImplicitConversionOverhead();

    /// @brief 演示 Latin-1 vs UTF-8 处理中文字符时的乱码陷阱。
    static void demoLatin1VsUtf8Pitfall();
};
