# IIMAS — 智能化信息管理分析系统

基于 C++20 的学生管理系统，采用四层分离架构（表现层 / 应用层 / 领域层 / 基础设施层），实践 SOLID 原则与 Repository 模式。

## 技术栈

| 层级 | 技术 |
|------|------|
| 后端语言 | C++20 |
| HTTP 框架 | [Crow](https://github.com/CrowCpp/Crow) (单 header 异步框架) |
| 数据库 | SQLite + [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp) |
| AI 推理 | [llama.cpp](https://github.com/ggerganov/llama.cpp) (本地 LLM) |
| 安全 | OpenSSL (SHA-256 密码哈希) |
| 导出 | OpenXLSX (Excel 生成) |
| 前端 | Vue 3 + Element Plus (CDN 加载，单文件 SPA) |
| 构建 | CMake 3.10+ |

## 架构

```
Presentation Layer   Crow HTTP 路由 (10 个文件) + CORS + Token 认证中间件
        |  依赖接口
Application Layer    StudentService / GroupService / SAGService / UserService
        |  依赖接口
Domain Layer         Model 结构体 + IRepository<T> 模板接口
        |  实现接口
Infrastructure Layer Repository 类 (SQLiteCpp) + llama.cpp + OpenXLSX
```

**关键设计决策：**

- **依赖倒置** — Service 层只依赖 `IRepository<T>` 接口，不依赖具体 Repository 类。构造函数注入接口引用。
- **Repository 模式** — 通过模板接口 `IRepository<T>` 为 Student / Group / SAG / User 四种实体提供统一的 CRUD 抽象。
- **Token 认证** — 登录成功后生成 token，后续请求通过 Bearer token 验证，用 `shared_mutex` 保证线程安全。
- **统一响应** — `ApiResponse` 工厂类封装 JSON 响应格式，避免路由层散落手写 JSON。

```
main.cpp 初始化链
  SQLite DB → 4 个 Repository → 4 个 Service → IIMAS_API → Crow 启动
```

## 数据库表

| 表名 | 字段 |
|------|------|
| `students` | id, student_id (UNIQUE), name, gender, class, subject, phone, remark, create_time |
| `groups` | id, group_name, description, create_time |
| `student_group` | id, student_id, group_id, remark, join_time |
| `users` | id, user_name, password (SHA-256) |

## 快速开始

### 前置条件

- C++20 编译器 (GCC 11+ / Clang 14+)
- CMake 3.10+
- 预编译静态库已放置在 `lib/` 目录

### 构建

```bash
cd build
cmake ..
make -j$(nproc)
```

### 运行

```bash
# 正常运行 (监听 3002 端口，多线程)
./bin/IIMAS_refactoring

# 调试模式 (打印 Crow DEBUG 日志)
./bin/IIMAS_refactoring debug
```

前端访问：打开 `html/index.html` 或通过 Crow 静态文件服务访问。

## API 路由概览

| 分类 | 路由 | 方法 | 说明 |
|------|------|------|------|
| 认证 | `/api/login/` | POST | 用户登录 |
| 认证 | `/api/logout/` | POST | 用户登出 |
| 搜索 | `/api/exactSearchStudent/` | POST | 精确搜索学生 |
| 搜索 | `/api/fuzzySearchStudent/` | POST | 模糊搜索学生 |
| 搜索 | `/api/searchGroup/` | POST | 搜索分组 |
| 搜索 | `/api/searchSAG/` | POST | 搜索学生-分组关联 |
| 添加 | `/api/addStudent/` | POST | 批量添加学生 |
| 添加 | `/api/addGroup/` | POST | 批量添加分组 |
| 添加 | `/api/addSAG/fromAll/` | POST | 从已有学生添加到分组 |
| 添加 | `/api/addSAG/new/` | POST | 新学生直接添加到分组 |
| 删除 | `/api/deleteStudent/` | POST | 删除学生（含关联） |
| 删除 | `/api/deleteGroup/` | POST | 删除分组（含关联） |
| 删除 | `/api/deleteSAG/` | POST | 从分组移除学生 |
| 修改 | `/api/updateStudent/<mode>` | POST | 修改学生 (single/batch) |
| 修改 | `/api/updateGroup/` | POST | 修改分组 |
| 修改 | `/api/updateSAG/` | POST | 修改关联备注 |
| 排序 | `/api/orderStudent/` | POST | 学生排序 |
| 排序 | `/api/orderGroup/` | POST | 分组排序 |
| AI | `/api/chat/` | POST | AI 对话 |
| Excel | `/api/export/*` | POST | Excel 导出 |

> 除 `/api/login/` 外，所有 API 请求需要在 Header 中携带 `Authorization: Bearer <token>`。

## 目录结构

```
.
├── CMakeLists.txt              # 构建配置
├── README.md
├── include/                    # 头文件
│   ├── model.hpp               # Domain 模型 + 枚举 + 映射表
│   ├── IRepository.hpp         # Repository 模板接口
│   ├── ApiResponse.hpp         # 统一 JSON 响应
│   ├── TokenStore.hpp          # 线程安全 Token 单例
│   ├── Crow_API.hpp            # IIMAS_API 类 + 中间件
│   ├── *Service.hpp            # 4 个 Service 声明
│   └── *Repository.hpp         # 4 个 Repository 声明
├── src/                        # 源文件
│   ├── main.cpp                # 入口 + 初始化链
│   ├── Crow_API.cpp            # 应用主类
│   ├── *Routes.cpp             # 路由文件 (按功能拆分)
│   ├── *Service.cpp            # 4 个 Service 实现
│   ├── *Repository.cpp         # 4 个 Repository 实现
│   └── ...
├── html/
│   └── index.html              # 前端 SPA
└── lib/                        # 预编译静态库
```

