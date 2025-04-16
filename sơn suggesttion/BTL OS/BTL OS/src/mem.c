#include "mem.h"
#include "stdlib.h"
#include "string.h"
#include <pthread.h>
#include <stdio.h>

// Mảng RAM mô phỏng bộ nhớ vật lý thật
static BYTE _ram[RAM_SIZE];

// Cấu trúc lưu thông tin trạng thái của từng frame trong RAM
static struct {
	uint32_t proc;	// ID của tiến trình đang sử dụng frame này
	int index;	// Số thứ tự trang của tiến trình trong danh sách các trang nó đang sử dụng
	int next;	// Chỉ số trang tiếp theo trong danh sách (dạng linked list); -1 nếu là trang cuối
} _mem_stat[NUM_PAGES];

// Khóa mutex bảo vệ truy cập RAM
static pthread_mutex_t mem_lock;

// Hàm khởi tạo bộ nhớ: đặt lại RAM và bảng trạng thái
void init_mem(void) {
	memset(_mem_stat, 0, sizeof(*_mem_stat) * NUM_PAGES);
	memset(_ram, 0, sizeof(BYTE) * RAM_SIZE);
	pthread_mutex_init(&mem_lock, NULL);
}

// Lấy offset từ địa chỉ ảo
static addr_t get_offset(addr_t addr) {
	return addr & ~((~0U) << OFFSET_LEN);
}

// Lấy chỉ số phân đoạn (segment index)
static addr_t get_first_lv(addr_t addr) {
	return addr >> (OFFSET_LEN + PAGE_LEN);
}

// Lấy chỉ số trang (page index) trong phân đoạn
static addr_t get_second_lv(addr_t addr) {
	return (addr >> OFFSET_LEN) - (get_first_lv(addr) << PAGE_LEN);
}

// Tìm bảng ánh xạ chuyển tiếp (trans_table) từ bảng phân đoạn
static struct trans_table_t * get_trans_table(addr_t index, struct page_table_t * page_table) {
	// Duyệt qua bảng phân đoạn để tìm phân đoạn có index tương ứng
	for (int i = 0; i < page_table->size; i++) {
		if (page_table->table[i].v_index == index) {
			return page_table->table[i].next_lv; // Trả về bảng ánh xạ chuyển tiếp
		}
	}
	return NULL; // Không tìm thấy
}

// Hàm dịch địa chỉ ảo sang địa chỉ vật lý
static int translate(addr_t virtual_addr, addr_t *physical_addr, struct pcb_t *proc) {
	// Bóc tách địa chỉ ảo thành offset, chỉ số phân đoạn, chỉ số trang
	addr_t offset = get_offset(virtual_addr);
	addr_t first_lv = get_first_lv(virtual_addr);
	addr_t second_lv = get_second_lv(virtual_addr);

	// Tìm bảng ánh xạ của phân đoạn
	struct trans_table_t *trans_table = get_trans_table(first_lv, proc->page_table);
	if (trans_table == NULL) return 0;

	// Duyệt bảng ánh xạ để tìm frame tương ứng với trang
	for (int i = 0; i < trans_table->size; i++) {
		if (trans_table->table[i].v_index == second_lv) {
			*physical_addr = (trans_table->table[i].p_index << OFFSET_LEN) | offset;
			return 1;
		}
	}
	return 0; // Không tìm thấy trang tương ứng
}

// Hàm cấp phát vùng nhớ ảo mới cho tiến trình
addr_t alloc_mem(uint32_t size, struct pcb_t *proc) {
	pthread_mutex_lock(&mem_lock);
	addr_t ret_mem = 0;

	// Tính số trang cần cấp phát
	uint32_t num_pages = (size % PAGE_SIZE) ? size / PAGE_SIZE + 1 : size / PAGE_SIZE;
	int mem_avail = 0;


	if (mem_avail) {
		// Cập nhật địa chỉ bắt đầu cấp phát
		ret_mem = proc->bp;
		proc->bp += num_pages * PAGE_SIZE;

	}

	pthread_mutex_unlock(&mem_lock);
	return ret_mem;
}

// Hàm giải phóng bộ nhớ (chưa cài đặt)
int free_mem(addr_t address, struct pcb_t *proc) {
	return 0;
}

// Đọc dữ liệu từ địa chỉ ảo
int read_mem(addr_t address, struct pcb_t *proc, BYTE *data) {
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		*data = _ram[physical_addr];
		return 0;
	} else {
		return 1;
	}
}

// Ghi dữ liệu vào địa chỉ ảo
int write_mem(addr_t address, struct pcb_t *proc, BYTE data) {
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		_ram[physical_addr] = data;
		return 0;
	} else {
		return 1;
	}
}

// In ra trạng thái bộ nhớ vật lý và dữ liệu đang lưu
void dump(void) {
	for (int i = 0; i < NUM_PAGES; i++) {
		if (_mem_stat[i].proc != 0) {
			printf("%03d: ", i);
			printf("%05x-%05x - PID: %02d (idx %03d, nxt: %03d)\n",
			       i << OFFSET_LEN,
			       ((i + 1) << OFFSET_LEN) - 1,
			       _mem_stat[i].proc,
			       _mem_stat[i].index,
			       _mem_stat[i].next);
			for (int j = i << OFFSET_LEN; j < ((i + 1) << OFFSET_LEN) - 1; j++) {
				if (_ram[j] != 0) {
					printf("\t%05x: %02x\n", j, _ram[j]);
				}
			}
		}
	}
}
