-- 定义一个缓存表
local cache = {}

-- 定义一个辅助函数，用于获取指定模块的内存范围
local function getMemoryRanges(moduleName, state, index)
    -- 检查缓存
    local cacheKey = string.format("%s_%s_%d", moduleName, state, index)
    if cache[cacheKey] then
        return cache[cacheKey]
    end

    local ranges = gg.getRangesList(moduleName .. '*$')
    local filteredRanges = {}
    for _, range in ipairs(ranges) do
        if string.find(state, range.state) then
            table.insert(filteredRanges, range)
        end
    end

    -- 缓存结果
    cache[cacheKey] = filteredRanges
    return filteredRanges
end

-- 定义一个辅助函数，用于将状态转换为权限
local function stateToPerm(state)
    local stateMap = {
        Cd = "rw-p",
        Cdro = "r--p",
        Cdx = "rwxp",
        Xa = "r-xp"
    }
    return stateMap[state]
end

-- 定义一个辅助函数，用于处理指针链并获取最终地址
local function processPointerChain(range, pointerList)
    local addr = (range.start + pointerList[1]) & 0xFFFFFFFFFF
    for i = 2, #pointerList do
        local values = gg.getValues({ { address = addr, flags = 32 } })
        addr = (values[1].value + pointerList[i]) & 0xFFFFFFFFFF
    end
    return addr
end

-- 定义设置指针值的函数
local function setPointerValue(moduleName, state, index, pointerList, value, valueType, freeze)
    local ranges = getMemoryRanges(moduleName, state, index)
    local filteredRanges = {}
    if state == "Cb" then
        filteredRanges = ranges
    else
        local perm = stateToPerm(state)
        for _, range in ipairs(ranges) do
            if range.type == perm then
                table.insert(filteredRanges, range)
            end
        end
    end

    if not filteredRanges[index] then
        local error = string.format("error:module %s[%s][%d] is not found", moduleName, state, index)
        print(error)
        return
    end

    local range = filteredRanges[index]
    local addr = processPointerChain(range, pointerList)
    local valueTable = { { address = addr, flags = valueType, freeze = freeze } }
    if value then
        valueTable[1].value = value
        gg.setValues(valueTable)
    end
    if freeze then
        gg.addListItems(valueTable)
    end
end

-- 定义获取指针地址的函数
local function getPointerAddress(moduleName, state, index, pointerList)
    local ranges = getMemoryRanges(moduleName, state, index)
    local filteredRanges = {}
    if state == "Cb" then
        filteredRanges = ranges
    else
        local perm = stateToPerm(state)
        for _, range in ipairs(ranges) do
            if range.type == perm then
                table.insert(filteredRanges, range)
            end
        end
    end

    if not filteredRanges[index] then
        local error = string.format("error:module %s[%s][%d] is not found", moduleName, state, index)
        print(error)
        return
    end

    local range = filteredRanges[index]
    return processPointerChain(range, pointerList)
end

-- ============================================
-- 使用说明:
-- ============================================

-- 1. 获取指针地址并添加到列表:
--    gg.addListItems({{address=getPointerAddress(...), flags=数据类型}})

-- 2. 设置指针值:
--    setPointerValue(模块名, 状态, 索引, 偏移量列表, 值, 数据类型, 是否冻结)

-- ============================================
-- 函数参数说明:
-- ============================================

-- getPointerAddress(moduleName, state, index, pointerList)
--    参数:
--      moduleName  (string)  - 模块名称, 如: "libil2cpp.so", "libunity.so"
--      state       (string)  - 内存区域状态: "Cd"(读写), "Cb"(BSS), "Cdx"(可执行读写), "Cdro"(只读), "Xa"(可执行)
--      index       (number)  - 模块索引, 从1开始计数
--      pointerList (table)   - 偏移量列表, 如: {0x278, 0x130, 0x28}
--    返回: (number) 最终计算出的内存地址

-- setPointerValue(moduleName, state, index, pointerList, value, valueType, freeze)
--    参数:
--      moduleName  (string)  - 模块名称, 如: "libil2cpp.so", "libunity.so"
--      state       (string)  - 内存区域状态: "Cd"(读写), "Cb"(BSS), "Cdx"(可执行读写), "Cdro"(只读), "Xa"(可执行)
--      index       (number)  - 模块索引, 从1开始计数
--      pointerList (table)   - 偏移量列表, 如: {0x278, 0x130, 0x28}
--      value       (number)  - 要设置的值, nil表示不设置值
--      valueType   (number)  - 数据类型:
--                               127 = Auto (自动)
--                               1   = Byte (字节)
--                               64  = Double (双精度浮点数)
--                               4   = Dword (双字)
--                               16  = Float (浮点数)
--                               32  = Qword (四字)
--                               2   = Word (字)
--                               8   = Xor (异或)
--      freeze      (boolean) - 是否冻结该地址, true冻结, false不冻结
--    返回: 无

-- ============================================
-- 使用示例:
-- ============================================

-- 示例1: 获取地址并添加到列表(Dword类型)
-- gg.addListItems({{address=getPointerAddress("libil2cpp.so", "Cd", 1, {0x278, 0x130, 0x28}), flags=4}})

-- 示例2: 设置值为999(Dword类型), 不冻结
-- setPointerValue("libil2cpp.so", "Cd", 1, {0x278, 0x130, 0x28}, 999, 4, false)

-- 示例3: 设置值为123.45(Float类型), 并冻结
-- setPointerValue("libunity.so", "Cd", 1, {0x59358, 0x130, 0x48}, 123.45, 16, true)

-- ============================================
-- 生成的脚本内容:
-- ============================================

gg.addListItems({{address=getPointerAddress("libgame.so", "Cd", 1, {0x10, 0x38, 0x2ec}),flags=4}})
setPointerValue("libgame.so", "Cd", 1, { 0x10,0x38,0x2ec }, 250, 16, false)

print('该脚本为fscan自动生成')
