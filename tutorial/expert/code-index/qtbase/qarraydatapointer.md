---
title: QArrayDataPointer COW 智能指针
description: QArrayDataPointer 的三成员结构、完整生命周期（拷贝构造/detach/析构）、copy-and-swap 赋值、fromRawData 视图、emplace 快路径。
---

# QArrayDataPointer COW 智能指针

> 本索引收录 Qt 6.9.1 源码中 QArrayDataPointer 相关的已验证证据。QArrayDataPointer 是 QString/QByteArray/QList 的核心 COW 智能指针。

## 三成员结构

源码文件：`qtbase/src/corelib/tools/qarraydatapointer.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Data*d / T* ptr / qsizetype size | :516-518 | `Data *d; T *ptr; qsizetype size;` | d 指向 QArrayData 头部，ptr 指向实际数据首元素（prepend 时可能不紧跟头部），size 是有效元素数。 |

## 拷贝构造（O(1) 浅拷贝）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 浅拷贝 d/ptr/size + ref() | :37-40 | `d(other.d), ptr(other.ptr), size(other.size) { ref(); }` | 指针赋值 + 原子 ref+1，不管数据多大都是 O(1)。 |

## ref/deref（null-safety）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ref() 只在 d 非 null 时调 d->ref() | :451 | `void ref() noexcept { if (d) d->ref(); }` | fromRawData 场景 d==nullptr，安全跳过。 |
| deref() 在 d==null 时返回 true | :452 | `bool deref() noexcept { return !d \|\| d->deref(); }` | d==null 不释放（fromRawData 不拥有数据）。 |
| needsDetach() 额外处理 d==nullptr | :456 | `return !d \|\| d->needsDetach();` | fromRawData 的 d=nullptr 也视为需要 detach。 |

## detach 机制

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| detach() 检查 needsDetach() 后调 reallocateAndGrow | :142-145 | `if (needsDetach()) reallocateAndGrow(QArrayData::GrowsAtEnd, 0, old);` | 惰性触发：ref_==1 时什么都不做。 |
| reallocateAndGrow：分配新块→拷贝→swap→旧 deref | :218-250 | `allocateGrow → copyAppend → swap(dp)` | COW 核心流程。swap 后旧数据由临时对象析构 deref。 |

## 赋值（copy-and-swap）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| operator= 使用 copy-and-swap | :69-73 | `QArrayDataPointer tmp(other); this->swap(tmp);` | 自赋值安全：tmp 拷贝自身 ref+1，swap 后 tmp 析构 deref 一次，净效果不变。 |

## 析构

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| deref() 返回 false 时 destroyAll + deallocate | :106-111 | `if (!deref()) { (*this)->destroyAll(); Data::deallocate(d); }` | ref_ 归零：先逐个析构元素，再释放整块内存。 |

## fromRawData 视图

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| fromRawData 创建 d=nullptr 的视图 | :63 | `static QArrayDataPointer fromRawData(const T *rawData, qsizetype length)` | 不拥有数据。非 const 操作触发完整内存分配和数据拷贝。 |

## 容器中的 detach 触发

### QString

源码文件：`qtbase/src/corelib/text/qstring.h` / `qstring.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 拷贝构造浅拷贝 d | qstring.h:1340-1341 | `QString(const QString &other) noexcept : d(other.d) { }` | O(1)。 |
| detach() 用自己的 reallocData | qstring.h:1334-1335 | `if (d.needsDetach()) reallocData(d.size, QArrayData::KeepSize);` | 需要处理 null 终止符。 |
| reallocData()：alloc==0 回 _empty，需要 detach 时 memcpy + 写 null 终止符 | qstring.cpp:2781-2802 | `if (!alloc) { d = fromRawData(&_empty, 0); return; }` ... `memcpy` ... `dd.data()[dd.size] = 0;` | 空字符串不分配堆内存；深拷贝后手动写 null 终止符。 |
| 非 const data()/begin()/end() 调用 detach() | qstring.h:1326-1330 | `detach(); return ...;` | const 版本不触发。 |
| operator[] 非 const 通过 data() 间接触发 | qstring.h:1431-1432 | `return data()[i];` | const 版本走 at()，不触发。 |
| 默认构造 constexpr，不分配 | qstring.h:1410 | `constexpr QString() noexcept {}` | d.d==nullptr，第一次非 const 操作才分配。 |

### QByteArray

源码文件：`qtbase/src/corelib/text/qbytearray.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 拷贝构造浅拷贝 d | :634-635 | `QByteArray(const QByteArray &a) noexcept : d(a.d) {}` | 与 QString 对称。 |
| 非 const data() 触发 detach | :616-620 | `detach(); return d.data();` | const 版本不触发。 |
| constData() 等价 data() const | :124 | `const char *constData() const noexcept { return data(); }` | 别名，不触发 detach。 |

### QList

源码文件：`qtbase/src/corelib/tools/qlist.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| detach() 委托 d.detach() | :457 | `void detach() { d.detach(); }` | 不需要自己的 reallocData（无 null 终止符）。 |
| 非 const begin()/end() 触发 detach | :656-657 | `iterator begin() { detach(); return ...; }` | const 版本 noexcept 且不触发。 |
| operator[] 非 const 通过 data() 触发 | :482-488 | `// don't detach() here, we detach in data below: return data()[i];` | const 版本走 at()。 |
| replace() 用 d.detach(&oldData) 保留旧引用 | :574-580 | `DataPointer oldData; d.detach(&oldData); d.data()[i] = t;` | 防止赋值期间旧数据被提前释放。 |

### QList emplace 快路径

源码文件：`qtbase/src/corelib/tools/qarraydataops.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| emplace 有 freeSpace 快路径：不需要 detach 且有空位时直接 placement new | :142-167 | `if (!detach && freeSpaceAtEnd()) { new (end()) T(...); return; }` | 避免不必要的内存分配和 detach。 |
