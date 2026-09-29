/// @file    worker.js
/// @brief   WorkerScript 后台线程 JavaScript 文件。
///
/// 在独立线程中执行素数计算，避免阻塞 QML 主线程 UI 渲染。
/// WorkerScript.onMessage 接收主线程消息，计算完毕后通过
/// sendMessage() 将结果回传。

/// @brief 消息处理入口，由主线程 WorkerScript.sendMessage() 触发。
/// @param message 消息对象，包含 action 和 limit 字段。
/// @note 所有计算在此函数内执行，不会阻塞 QML 渲染线程。
WorkerScript.onMessage = function(message) {
    if (message.action === "compute") {
        var limit = message.limit;
        var primes = computePrimes(limit);
        WorkerScript.sendMessage({
            action: "result",
            count: primes.length,
            limit: limit
        });
    }
};

/// @brief 埃拉托斯特尼筛法计算 limit 以内的所有素数。
/// @param limit 筛选上界（正整数）。
/// @return 素数数组。
/// @note 使用经典筛法，时间复杂度 O(n log log n)，适合教学演示。
function computePrimes(limit) {
    if (limit < 2) {
        return [];
    }
    // 初始化标记数组，true 表示尚未被筛除
    var sieve = new Array(limit + 1).fill(true);
    sieve[0] = false;
    sieve[1] = false;

    var sqrtLimit = Math.sqrt(limit);
    for (var i = 2; i <= sqrtLimit; ++i) {
        if (sieve[i]) {
            // 将 i 的所有倍数标记为非素数
            for (var j = i * i; j <= limit; j += i) {
                sieve[j] = false;
            }
        }
    }

    var primes = [];
    for (var k = 2; k <= limit; ++k) {
        if (sieve[k]) {
            primes.push(k);
        }
    }
    return primes;
}
