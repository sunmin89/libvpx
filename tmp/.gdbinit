# 保存历史命令
set history filename .gdb_history
set history save on

# 退出时不显示提示信息
set confirm off

# 按照派生类型打印对象
set print object on

# 打印数组的索引下标
set print array-indexes off

# 每行打印一个结构体成员
set print pretty on

#b main
alias ddw='target remote localhost:1234'