# Qt元对象系统分析小记



​	笔者打算深入研究一下Qt元对象，这里将分析结果记录一下：

## 为什么是Qt元对象系统

​	第一个问题：什么是元对象？

​	首先，原生的C++，是没有反射这个概念的。或者说，我们没有办法使用标准直接快速的获取类的所有信息：比如说快速取到表达类名的字符串，类方法的字符串等等。即使广大编译器有了各种各样的支持，但是C++的实现自由就导致了每一个编译器的实现方式不一样，我们书写的程序非常难以实现通用。

​	Qt提供了更为强大的元对象（Meta-Object）[系统机制](https://zhida.zhihu.com/search?content_id=101752235&content_type=Article&match_order=1&q=系统机制&zhida_source=entity)来实现动态获取类型信息，以及扩展我们的原生C++：

| **功能**       | **描述**                                                     |
| -------------- | ------------------------------------------------------------ |
| **元对象类**   | 每个`QObject`子类都有一个对应的元对象类，提供对象的名称、属性、信号和槽的信息。 |
| **信号和槽**   | 支持对象之间的通信机制，信号发送时可以连接到一个或多个槽，处理相应的事件。 |
| **动态属性**   | 可以在运行时为对象添加、修改或删除属性，增加了对象的灵活性。 |
| **类型信息**   | 提供运行时类型识别功能，允许检查对象的类型并进行相应处理。   |
| **Qt MOC**     | 元对象编译器，解析信号和槽，生成相应的代码以支持元对象功能。 |
| **反射**       | 允许在运行时获取对象的元信息，比如可以列出所有属性、信号和槽等。 |
| **国际化支持** | 通过元对象系统，可以方便地处理字符串的翻译，支持多语言应用。 |
| **对象树**     | 通过父子关系管理对象的生命周期，父对象可以自动删除子对象。   |

​	上面这个表格记录了Qt元对象系统的主要功能和特性。现在，我们对Qt元对象系统有了一个大致的Framework了！但是只知道这些貌似跟没说一样。我们下一步来深入Qt元对象系统。

## 关于Qt元对象的进一步分析

​	Qt 的元对象系统提供了用于**对象间通信的信号和槽机制、运行时类型信息和动态属性系统。** 

​	元对象系统基于三件非常重要的东西： 

- QObject 类为可以利用元对象系统的对象提供了基类。 
- Q_OBJECT 宏用于启用元对象功能，例如动态属性、信号和槽。 
- 元对象编译器 (moc) 为每个 QObject 子类提供实现元对象功能所需的代码。 

moc 工具读取 C++ 源文件。如果它发现一个或多个包含 Q_OBJECT 宏的类声明，它会生成另一个 C++ 源文件，其中包含每个类的元对象代码。生成的源文件要么被 #include 到类的源文件中，要么更常见的是编译并与类的实现链接。 除了提供用于对象间通信的信号和槽机制（引入系统的主要原因）之外，元对象代码还提供以下附加功能：

- QObject::metaObject() 返回与该类相关的元对象。
-  QMetaObject::className() 在运行时以字符串形式返回类名，无需通过 C++ 编译器支持本机运行时类型信息 (RTTI)。 
- QObject::inherits() 函数返回某个对象是否是继承 QObject 继承树中指定类的类的实例。 
- QObject::tr() 翻译字符串以实现国际化。 
- QObject::setProperty() 和 QObject::property() 通过名称动态设置和获取属性。 
- QMetaObject::newInstance() 构造该类的新实例。

​	现在我们大概可以猜到：Qt实现元对象系统其实就是依靠三个部分：一个QObject原生类提供接口，Q_OBJECT也就是内部封装的QMetaObject实际上是就是实现Qt元对象主要功能的。下一步就是moc编译器实际上生成源代码。三者缺一不可。

```
/* qmake ignore Q_OBJECT */
#define Q_OBJECT \
public: \
    QT_WARNING_PUSH \
    Q_OBJECT_NO_OVERRIDE_WARNING \
    static const QMetaObject staticMetaObject; \
    virtual const QMetaObject *metaObject() const; \
    virtual void *qt_metacast(const char *); \
    virtual int qt_metacall(QMetaObject::Call, int, void **); \
    QT_TR_FUNCTIONS \
private: \
    Q_OBJECT_NO_ATTRIBUTES_WARNING \
    Q_DECL_HIDDEN_STATIC_METACALL static void qt_static_metacall(QObject *, QMetaObject::Call, int, void **); \
    QT_WARNING_POP \
    struct QPrivateSignal { explicit QPrivateSignal() = default; }; \
    QT_ANNOTATE_CLASS(qt_qobject, "")
```

​	qobject_cast是一个类似于dynamic_cast的函数，当然他是读取元信息进行转换的！对于转换不成功行为也是返回空！

```
if (QLabel *label = qobject_cast<QLabel *>(obj)) {
        label->setText(tr("Ping"));
    } else if (QPushButton *button = qobject_cast<QPushButton *>(obj)) {
        button->setText(tr("Pong!"));
}
```

​	虽然可以使用 QObject 作为基类，而无需 Q_OBJECT 宏和元对象代码，但如果不使用 Q_OBJECT 宏，则信号和槽以及此处描述的其他功能都将不可用。从元对象系统的角度来看，没有元代码的 QObject 子类相当于其具有元对象代码的最近祖先。

​	这意味着，例如，QMetaObject::className() 将不会返回您的类的实际名称，而是返回此祖先的类名。 **因此，强烈建议 QObject 的所有子类都使用 Q_OBJECT 宏，无论它们是否实际使用信号、槽和属性**。

## QMetaObjectAPI列表

| API                                                          | 描述                                 |
| ------------------------------------------------------------ | ------------------------------------ |
| `const QMetaObject *metaObject()`                            | 返回与对象关联的元对象。             |
| `QString className() const`                                  | 返回类名的字符串表示。               |
| `bool inherits(const char *name) const`                      | 检查对象是否是指定类或其子类的实例。 |
| `int methodCount() const`                                    | 返回方法的数量。                     |
| `QMetaMethod method(int index) const`                        | 返回指定索引的元方法。               |
| `int propertyCount() const`                                  | 返回属性的数量。                     |
| `QMetaProperty property(int index) const`                    | 返回指定索引的元属性。               |
| `int enumeratorCount() const`                                | 返回枚举的数量。                     |
| `QMetaEnum enumerator(int index) const`                      | 返回指定索引的元枚举。               |
| `QMetaObject *superClass() const`                            | 返回超类的元对象。                   |
| `QMetaObject::Constructor constructor(int index)`            | 返回指定索引的构造函数。             |
| `QMetaMethod method(const char *signature)`                  | 根据签名返回对应的元方法。           |
| `QMetaProperty property(const char *name)`                   | 根据属性名返回对应的元属性。         |
| `QMetaObject::PropertyFlags propertyFlags(const char *name)` | 返回属性的标志。                     |
| `QVariant property(const char *name) const`                  | 获取指定属性名的值。                 |
| `bool setProperty(const char *name, const QVariant &value)`  | 设置指定属性名的值。                 |

​	关于使用上，需要记得

1. 继承QObject
2. 使用Q_OBJECT宏

满足这些就可以使用**Qt元对象类信息的使用，但是，属性信息不行！**到底如何，请听下回分解



# Reference

[The Meta-Object System | Qt Core 6.8.0](https://doc.qt.io/qt-6/metaobjects.html)

[Qt中的元对象系统（Meta-Object System） - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/61303678)

[QMetaObject Struct | Qt Core 6.8.0](https://doc.qt.io/qt-6/qmetaobject.html)

[Qt's Property System](https://doc.qt.io/qt-6/properties.html)



表格：所有API：

| API                                                          | 描述                                 |
| ------------------------------------------------------------ | ------------------------------------ |
| `QMetaObject()`                                              | 构造函数。                           |
| `const QMetaObject *metaObject() const`                      | 返回与对象关联的元对象。             |
| `QString className() const`                                  | 返回类名的字符串表示。               |
| `bool inherits(const char *name) const`                      | 检查对象是否是指定类或其子类的实例。 |
| `int methodCount() const`                                    | 返回方法的数量。                     |
| `QMetaMethod method(int index) const`                        | 返回指定索引的元方法。               |
| `int propertyCount() const`                                  | 返回属性的数量。                     |
| `QMetaProperty property(int index) const`                    | 返回指定索引的元属性。               |
| `int enumeratorCount() const`                                | 返回枚举的数量。                     |
| `QMetaEnum enumerator(int index) const`                      | 返回指定索引的元枚举。               |
| `QMetaObject *superClass() const`                            | 返回超类的元对象。                   |
| `QMetaMethod method(const char *signature) const`            | 根据签名返回对应的元方法。           |
| `QMetaProperty property(const char *name) const`             | 根据属性名返回对应的元属性。         |
| `QMetaObject::PropertyFlags propertyFlags(const char *name) const` | 返回属性的标志。                     |
| `QVariant property(const char *name) const`                  | 获取指定属性名的值。                 |
| `bool setProperty(const char *name, const QVariant &value)`  | 设置指定属性名的值。                 |
| `QMetaObject::Constructor constructor(int index) const`      | 返回指定索引的构造函数。             |
| `int constructorCount() const`                               | 返回构造函数的数量。                 |
| `QMetaMethod constructor(int index) const`                   | 返回指定索引的构造函数元方法。       |
| `QMetaMethod signal(int index) const`                        | 返回指定索引的信号。                 |
| `int signalCount() const`                                    | 返回信号的数量。                     |
| `QMetaMethod slot(int index) const`                          | 返回指定索引的槽。                   |
| `int slotCount() const`                                      | 返回槽的数量。                       |
| `int indexOfMethod(const char *signature) const`             | 根据签名返回方法的索引。             |
| `int indexOfSignal(const char *signal) const`                | 根据信号返回信号的索引。             |
| `int indexOfSlot(const char *slot) const`                    | 根据槽返回槽的索引。                 |
| `int indexOfProperty(const char *name) const`                | 根据属性名返回属性的索引。           |
| `int indexOfEnumerator(const char *name) const`              | 根据枚举名返回枚举的索引。           |
| `QMetaEnum enumerator(const char *name) const`               | 根据枚举名返回对应的元枚举。         |