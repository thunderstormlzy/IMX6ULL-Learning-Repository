<!-- Source: 设备树下的platform驱动（8.22）.note | Exported from local Youdao database on 2026-10-06 -->

# 设备树下的platform驱动（8.22）

static const struct of_device_id leds_of_match[] = {
    { .compatible = "gpioled" }, /* 兼容属性 */
    { /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, leds_of_match);
static struct platform_driver leds_platform_driver = {
    .driver = {
        .name = "imx6ul-led",
        .of_match_table = leds_of_match,
    },
    .probe = leds_probe,
    .remove = leds_remove,
};
和platform设备驱动相比，将硬件描述放入设备树中，通过.compatible的兼容数据匹配设备树中同样兼容属性的硬件（
compatible 可以被同类设备复用，但要避免设备节点和资源定义冲突
）

MODULE_DEVICE_TABLE(of, ...)
将设备树匹配信息导出到模块元数据中，便于模块自动加载机制识别；其中 of 表示采用 OF（设备树）匹配方式。
