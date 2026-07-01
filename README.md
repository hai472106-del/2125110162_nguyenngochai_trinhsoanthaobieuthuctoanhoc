# Đề tài 6 — Trình Soạn Thảo Biểu Thức Toán Học

> ⚠️ Đổi tên thư mục `MSSV_HoTen_DeTai6` thành đúng mã số sinh viên và họ tên của bạn theo mẫu `MSSV_HoTen_DeTai` trước khi nộp.

Nhập biểu thức toán học, parse, tính giá trị, vẽ cây biểu thức (Expression Tree).

## Cấu trúc dữ liệu sử dụng

| Cấu trúc | Vai trò |
|---|---|
| **Stack** (tự cài đặt bằng danh sách liên kết, trong `structures.h`) | Thuật toán Shunting-Yard: chuyển Infix → Postfix; xây cây từ postfix; lưu lịch sử tính toán |
| **Queue** (tự cài đặt bằng danh sách liên kết, trong `structures.h`) | Hàng đợi token đầu ra trong quá trình Shunting-Yard |
| **Binary Tree** (Expression Tree, trong `structures.h` / `functions.cpp`) | Lưu cấu trúc biểu thức, in Infix/Prefix/Postfix, vẽ ASCII, tính giá trị |

`Stack<T>` và `Queue<T>` được cài đặt thủ công bằng danh sách liên kết đơn (không dùng `std::stack`/`std::queue`).

## Cấu trúc thư mục

```
MSSV_HoTen_DeTai/
├── src/
│   ├── main.cpp          # Chương trình chính + Menu điều hướng
│   ├── structures.h       # Khai báo: Stack<T>, Queue<T>, Token, ExpressionTree, History
│   └── functions.cpp      # Cài đặt: Tokenizer, Shunting-Yard, ExpressionTree, History
├── tests/
│   └── test_cases.cpp     # Chương trình test độc lập (>= 5 test case), có main() riêng
├── docs/
│   └── bao_cao.pdf        # (tuỳ chọn) báo cáo đề tài
└── README.md
```

## Chức năng chính

1. **Nhập biểu thức dạng Infix**: ví dụ `(3 + 4) * 2 - 1`
2. **Chuyển sang Postfix (RPN)**: thuật toán Shunting-Yard (Stack + Queue) → `3 4 + 2 * 1 -`
3. **Tính giá trị biểu thức**: duyệt Expression Tree, hỗ trợ biến
4. **Vẽ Expression Tree dạng ASCII**: cây nghiêng 90°, gốc bên trái, lá bên phải
5. **In 3 dạng Infix / Prefix / Postfix** trực tiếp từ cây
6. **Hỗ trợ biến**: gán `x=5` rồi tính `x^2 + 2*x + 1`
7. **Lịch sử tính toán** (dùng Stack): hiển thị các lần tính gần nhất trước

## Cách biên dịch và chạy

Yêu cầu: `g++` hỗ trợ `-std=c++17`.

### Chương trình chính

```bash
cd src
g++ -std=c++17 -Wall main.cpp functions.cpp -o expr_editor
./expr_editor
```

(Trên Windows: `g++ -std=c++17 -Wall main.cpp functions.cpp -o expr_editor.exe` rồi chạy `expr_editor.exe`)

### Chạy test case (độc lập, có `main()` riêng)

```bash
cd tests
g++ -std=c++17 -Wall -I../src test_cases.cpp ../src/functions.cpp -o run_tests
./run_tests
```

> Lưu ý: `main.cpp` và `test_cases.cpp` đều có hàm `main()` nên **không được biên dịch chung** trong cùng một lệnh `g++`. Biên dịch chương trình chính và chương trình test thành hai file thực thi riêng biệt như trên.

## Ví dụ sử dụng (chương trình chính)

```
Chọn chức năng: 1
Nhập biểu thức infix: (3 + 4) * 2 - 1
>> Postfix (RPN): 3 4 + 2 * 1 -

Chọn chức năng: 4
Expression Tree (nghiêng 90 độ, gốc bên trái):
   /--1
-
   |  /--2
   \--*
      |  /--4
      \--+
         \--3

Chọn chức năng: 3
>> Kết quả = 13
```

## Test case (`tests/test_cases.cpp`)

| # | Mô tả | Kết quả mong đợi |
|---|---|---|
| 1 | `(3 + 4) * 2 - 1` | 13 |
| 2 | Xây cây trực tiếp từ postfix `3 4 + 2 * 1 -` | 13 |
| 3 | `x^2 + 2*x + 1` với `x = 5` | 36 |
| 4 | `10 / (2 + 3)` | 2 |
| 5 | `2 ^ 3 ^ 2` (kết hợp phải: `2^(3^2)`) | 512 |
| 6 | `-5 + 3` (dấu trừ một ngôi) | -2 |
| 7 | `a*b + c` với nhiều biến | 10 |

Chạy `./run_tests` để chương trình tự in kết quả PASS/FAIL cho từng test và tổng kết cuối cùng.

## Xử lý lỗi

Chương trình bắt và thông báo lỗi (không crash) trong các trường hợp:
- Biểu thức chứa ký tự không hợp lệ
- Thiếu dấu `(` hoặc `)`
- Biểu thức postfix không hợp lệ (thừa/thiếu toán hạng)
- Chia cho 0
- Biến chưa được gán giá trị khi tính toán

## Ghi chú kỹ thuật

- `Stack<T>` và `Queue<T>` là template, cài đặt bằng danh sách liên kết đơn, O(1) cho push/pop/enqueue/dequeue.
- `ExpressionTree` quản lý bộ nhớ bằng deep-copy constructor và destructor giải phóng đệ quy.
- Dấu `-`/`+` đứng đầu biểu thức hoặc sau toán tử/`(` được nhận diện là dấu một ngôi (unary).
- Toán tử `^` kết hợp phải (right-associative) đúng chuẩn toán học.
