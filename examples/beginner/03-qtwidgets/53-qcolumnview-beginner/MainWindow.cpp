#include "MainWindow.h"

#include "ProductColumnView.h"

#include <QColumnView>
#include <QLabel>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QColumnView 综合演示 — 产品分类浏览器");
    resize(960, 560);
    initUi();
    populateProductData();
}

/// @brief 初始化界面
void MainWindow::initUi()
{
    auto *centralWidget = new QWidget;
    auto *mainLayout = new QVBoxLayout(centralWidget);

    // 多列级联视图
    m_columnView = new ProductColumnView;
    m_model = new QStandardItemModel(this);
    m_columnView->setModel(m_model);

    mainLayout->addWidget(m_columnView, 1);

    // 底部面包屑导航
    m_breadcrumbLabel = new QLabel("请选择分类");
    m_breadcrumbLabel->setStyleSheet(
        "padding: 6px 12px;"
        "background-color: #F8F9FA;"
        "border: 1px solid #E9ECEF;"
        "border-radius: 3px;"
        "color: #495057;"
        "font-size: 12px;");
    mainLayout->addWidget(m_breadcrumbLabel);

    setCentralWidget(centralWidget);

    // 点击时更新面包屑
    connect(m_columnView, &QColumnView::clicked,
            this, &MainWindow::onItemClicked);
}

/// @brief 构建三层产品分类树并填充数据
void MainWindow::populateProductData()
{
    // ============================================================
    // 第一层：产品大类
    // ============================================================
    auto *electronics = new QStandardItem("电子产品");
    auto *clothing = new QStandardItem("服装");
    auto *food = new QStandardItem("食品");
    m_model->appendRow({electronics, clothing, food});

    // ============================================================
    // 第二层 + 第三层：子分类 + 具体产品
    // ============================================================
    // 电子产品
    auto *phones = new QStandardItem("手机");
    auto *laptops = new QStandardItem("笔记本");
    auto *audio = new QStandardItem("音频设备");
    electronics->appendRow({phones, laptops, audio});

    addProduct(phones, "iPhone 16 Pro", "电子产品",
               8999.0, "Apple",
               "A18 Pro 芯片，钛金属边框，4800 万像素相机系统");
    addProduct(phones, "Pixel 9 Pro", "电子产品",
               6999.0, "Google",
               "Tensor G4 芯片，AI 相机，7 年系统更新");
    addProduct(phones, "Galaxy S25", "电子产品",
               7499.0, "Samsung",
               "骁龙 8 Elite，2 亿像素，Galaxy AI");

    addProduct(laptops, "MacBook Pro 14", "电子产品",
               14999.0, "Apple",
               "M4 Pro 芯片，Liquid Retina XDR 显示屏");
    addProduct(laptops, "ThinkPad X1 Carbon", "电子产品",
               11999.0, "Lenovo",
               "酷睿 Ultra 7，14 英寸 2.8K OLED");

    addProduct(audio, "AirPods Pro 3", "电子产品",
               1899.0, "Apple",
               "H3 芯片，自适应降噪，个性化空间音频");
    addProduct(audio, "WH-1000XM6", "电子产品",
               2999.0, "Sony",
               "旗舰降噪，30 小时续航，LDAC 高解析度");

    // 服装
    auto *tops = new QStandardItem("上装");
    auto *bottoms = new QStandardItem("下装");
    auto *shoes = new QStandardItem("鞋类");
    clothing->appendRow({tops, bottoms, shoes});

    addProduct(tops, "羊绒圆领毛衣", "服装",
               1299.0, "COS",
               "100% 山羊绒，经典圆领设计，手感柔软");
    addProduct(tops, "机能风冲锋衣", "服装",
               2599.0, "ARC'TERYX",
               "GORE-TEX 面料，防水透气，城市户外两用");

    addProduct(bottoms, "弹力修身西裤", "服装",
               899.0, "Massimo Dutti",
               "弹力面料，修身剪裁，商务休闲均可");
    addProduct(bottoms, "原色丹宁牛仔裤", "服装",
               699.0, "Levi's",
               "501 经典版型，日本冈山丹宁面料");

    addProduct(shoes, "Classic 老爹鞋", "服装",
               899.0, "New Balance",
               "990v6 系列，美产，ENCAP 中底缓震");
    addProduct(shoes, "Gel-Kayano 31", "服装",
               1299.0, "ASICS",
               "稳定系跑鞋，4D GUIDANCE 系统");

    // 食品
    auto *snacks = new QStandardItem("零食");
    auto *drinks = new QStandardItem("饮品");
    auto *fresh = new QStandardItem("生鲜");
    food->appendRow({snacks, drinks, fresh});

    addProduct(snacks, "70% 黑巧克力", "食品",
               49.0, "Lindt",
               "瑞士进口，可可含量 70%，口感丝滑");
    addProduct(snacks, "混合坚果仁", "食品",
               69.0, "每日坚果",
               "6 种坚果果干混合，独立小包装");

    addProduct(drinks, "冷萃咖啡液", "食品",
               89.0, "三顿半",
               "即溶即饮，6 种烘焙度可选");
    addProduct(drinks, "零糖气泡水", "食品",
               6.0, "元气森林",
               "0 糖 0 脂 0 卡，白桃味");

    addProduct(fresh, "秘鲁蓝莓", "食品",
               39.0, "佳沃",
               "当季新鲜空运，颗粒饱满甜度高");
    addProduct(fresh, "澳洲和牛 M5", "食品",
               298.0, "SILVER GRAIN",
               "雪花纹理 M5 等级，眼肉部位");
}

/// @brief 向分类节点添加一个带详情的产品条目
void MainWindow::addProduct(QStandardItem *parent,
                            const QString &name,
                            const QString &category,
                            double price,
                            const QString &brand,
                            const QString &description)
{
    auto *item = new QStandardItem(name);

    QVariantMap productInfo;
    productInfo["category"] = category;
    productInfo["price"] = price;
    productInfo["brand"] = brand;
    productInfo["description"] = description;
    item->setData(productInfo, Qt::UserRole);

    parent->appendRow(item);
}

/// @brief 点击条目时更新面包屑导航
void MainWindow::onItemClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        m_breadcrumbLabel->setText("请选择分类");
        return;
    }

    // 从当前节点回溯到根，构建路径
    QStringList pathParts;
    QModelIndex current = index;
    while (current.isValid()) {
        pathParts.prepend(
            m_model->data(current, Qt::DisplayRole).toString());
        current = current.parent();
    }

    m_breadcrumbLabel->setText(
        pathParts.join(" > "));
}
