#include <myos>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <unistd.h>
#include <string.h>
struct _FILE {
	uint8_t* buffer;
	uint64_t bufsize;
	uint64_t cursor;
	uint64_t fd;
	uint64_t flags;
	uint64_t limit;
	uint32_t ungetc_buf;
	uint32_t unget_count;
	uint8_t dirty;
};
extern "C" __attribute__((naked, section(".entry"))) void _start() {
	__asm__ __volatile__(
		"push rdi\n\t"
		"push rsi\n\t"
		"call _before_main\n\t"
		"pop rsi\n\t"
		"pop rdi\n\t"
		"call main\n\t"
		"mov rdi, rax\n\t"
		"call _after_main\n\t"
	);
}
uint64_t send_msg(uint64_t dest_pid, const msg_t* msg, bool is_block) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x04), "D"(0), "S"((uint64_t)msg), "d"(dest_pid), "c"((uint64_t)is_block)
		: "r11", "memory"
	);
	return ret; // return value in rax
}
uint64_t receive_msg(msg_t* msg) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x04), "D"(1), "S"((uint64_t)msg)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax (메시지 수신 성공 여부)
}
uint64_t get_tsc() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(0)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax
}
uint64_t get_cycles() {
	uint64_t ret;
	__asm__ __volatile__("int 0x80" : "=a"(ret) : "a"(0x07), "D"(7) : "rcx", "r11", "memory");
	return ret;
}
uint64_t get_memory_size() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(2)
		: "rcx", "r11", "memory"
	);
	return ret * 4096; // return value in rax (B 단위)
}
uint64_t get_used_memory_size() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(3)
		: "rcx", "r11", "memory"
	);
	return ret * 4096; // return value in rax (B 단위)
}
uint64_t get_free_memory_size() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(4)
		: "rcx", "r11", "memory"
	);
	return ret * 4096; // return value in rax (B 단위)
}
uint64_t get_process_count() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(5)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax (B 단위)
}
uint64_t get_max_process_count() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x07), "D"(6)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax (B 단위)
}
void shutdown() {
	__asm__ __volatile__(
		"int 0x80"
		:
	: "a"(-1ull), "D"(0)
		: "rcx", "r11", "memory"
		);
}
void reboot() {
	__asm__ __volatile__(
		"int 0x80"
		:
	: "a"(-1ull), "D"(1)
		: "rcx", "r11", "memory"
		);
}
void wait_for_msg() {
	__asm__ __volatile__(
		"int 0x81"
		:
		: "a"(0x04)
		: "rcx", "r11", "memory"
	);
}
void sleep(uint64_t ms) {
	__asm__ __volatile__(
		"int 0x81"
		:
	: "a"(32), "D"(ms)
		:"memory"
	);
}
uint64_t set_timer(uint64_t timeout_ms, uint64_t interval_ms) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x06), "D"(0x35), "S"(timeout_ms), "d"(interval_ms)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax (타이머 ID)
}
void cancel_timer(uint64_t timer_id) {
	__asm__ __volatile__(
		"int 0x80"
		:
	: "a"(0x06), "D"(0x0), "S"(timer_id)
		:"memory"
	);
}

SharedMem::SharedMem() : id(0), size(0) {}

void* SharedMem::create(uint64_t size) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(15), "D"(size), "S"(this)
		: "rcx", "r11", "memory"
	);
	if (ret == 0) return nullptr; // 실패
	return (void*)(uintptr_t)ret; // 반환값은 공유 메모리의 가상 주소
}

void* SharedMem::accept(uint64_t id) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(16), "D"(id), "S"(this)
		: "rcx", "r11", "memory"
	);
	if (ret == 0) return nullptr; // 실패
	return (void*)(uintptr_t)ret; // 반환값은 공유 메모리의 가상 주소
}
uint64_t get_ginfo(Ginfo* ginfo) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x05), "D"(0), "S"((uint64_t)ginfo)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax
}
uint64_t display_frame(void* frame_buffer) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(0x05), "D"(2), "S"((uint64_t)frame_buffer)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax
}
uint64_t SharedMem::get_id() const {
	return id;
}
uint64_t SharedMem::get_size() const {
	return size;
}
once_flag Window::ginfo_once_flag;
Ginfo Window::ginfo;
uint64_t Window::bytesPerPixel;
Window::Window(RECT rect, uint32_t style, uint32_t ex_style) : rect(rect) {
	call_once(ginfo_once_flag, [this]() {
		get_ginfo(&ginfo);
		switch (ginfo.format) { // Updated to use ginfo.format instead of ModeInfo->PixelFormat
		case GOP_PIXEL_FORMAT_RGBR:
		case GOP_PIXEL_FORMAT_BGRR:
			bytesPerPixel = 4;
			break;

		case GOP_PIXEL_FORMAT_BITMASK: {
			uint32_t mask = (uint32_t)ginfo.format;
			mask = (uint32_t)(ginfo.width | ginfo.height);
			uint32_t highest = 31;
			while (highest && ((mask >> highest) & 1) == 0)
				highest--;
			bytesPerPixel = ((highest + 1) + 7) / 8;
			break;
		}
		case GOP_PIXEL_FORMAT_BLT_ONLY:
		default:
			bytesPerPixel = 0;
			break;
		}
		});
	gbuf = gshm.create(rect.width * rect.height * bytesPerPixel);
	msg_t msg{
	.sender_pid = 1,
	.type = MSG_MAKE_WINDOW,
	.status = 0,
	.payload{ {gshm.get_id(),pack_u32(rect.x,rect.y), pack_u32(rect.width, rect.height), pack_u32(ex_style, style),0}},
	.timestamp = 0
	};
	this->rect.x = 0;
	this->rect.y = 0;
	uint64_t result = send_msg(0, &msg);
	printf("Code %d: Sent MSG_MAKE_WINDOW with shared memory id %d, result=%d\n", (int)msg.type, (int)gshm.get_id(), (int)result);
	wait_for_msg();
	msg_t response;
	receive_msg(&response);

}
void Window::rezorder() {
	msg_t msg{
		.sender_pid = 1,
		.type = MSG_REZORDER,
		.status = 0,
		.payload{ {0,0,0,0,0}},
		.timestamp = 0
	};
	uint64_t result = send_msg(0, &msg);
}
void Window::draw_frame(RECT rect) {
	if (gbuf == nullptr) return;
	msg_t msg{
		.sender_pid = 1,
		.type = MSG_DRAW_FRAME,
		.status = 0,
		.payload{ {gshm.get_id(),pack_u32(rect.x,rect.y), pack_u32(rect.width, rect.height)}},
		.timestamp = 0

	};
	uint64_t result = send_msg(0, &msg);
}
void Window::destroy() {
	msg_t msg{ .sender_pid = 1, .type = MSG_DESTROY_WINDOW, .status = 0, .payload{ {gshm.get_id(),0,0,0,0} }, .timestamp = 0 };
	send_msg(0, &msg);
}
Console::Console() : Console(get_pid()) {
}
Console::Console(pid_t pid) {
	console_pid = pid;
}
pid_t Console::get_pid() {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(29), "D"(1)
		: "rcx", "r11", "memory"
	);
	return ret; // return value in rax
}
pid_t Console::get_pid(pid_t pid) {
	return -1; // 나중에 msg로 구현
}
void Console::set_pid(pid_t pid) {
	uint64_t ret;
	__asm__ __volatile__(
		"int 0x80"
		: "=a"(ret)
		: "a"(29), "D"(0), "S"(pid)
		: "rcx", "r11", "memory"
	);
}
FILE* stdout;
FILE* stdin;
FILE* stderr;
extern "C" void _before_main() {
	stdout = (FILE*)malloc(sizeof(FILE));
	stdin = (FILE*)malloc(sizeof(FILE));
	stderr = (FILE*)malloc(sizeof(FILE));
	stdout->fd = 1; // 표준 출력
	stdout->buffer = (uint8_t*)malloc(1024); // 버퍼 할당
	stdout->bufsize = 1024;
	stdout->cursor = 0;
	stdout->flags = _IOLBF;
	stdout->dirty = 0;
	stdout->ungetc_buf = 0;
	stdout->unget_count = 0;
	stdout->limit = 1024;

	stdin->fd = 0; // 표준 입력
	stdin->buffer = (uint8_t*)malloc(1024); // 버퍼 할당
	stdin->bufsize = 1024;
	stdin->cursor = 0;
	stdin->flags = _IOFBF; // 버퍼링 없음
	stdin->dirty = 0;
	stdin->ungetc_buf = 0;
	stdin->unget_count = 0;
	stdin->limit = 0; // 아직 읽을게 없음

	stderr->fd = 2; // 표준 오류
	stderr->buffer = nullptr; // 버퍼 할당
	stderr->bufsize = 0;
	stderr->cursor = 0;
	stderr->flags = _IONBF; // 라인 버퍼링
	stderr->dirty = 0;
	stderr->ungetc_buf = 0;
	stderr->unget_count = 0;
	stderr->limit = 0;
}
extern "C" void _after_main(int exit_code) {
	fclose(stdin);
	exit(exit_code);
}
extern "C" void* __dso_handle = nullptr;


// ── Font (TrueType 파서 + 래스터라이저) 정의 ──
uint16_t Font::swap16(uint16_t v) { return (uint16_t)((v << 8) | (v >> 8)); }
uint32_t Font::swap32(uint32_t v) { return ((v >> 24) & 0xff) | ((v << 8) & 0xff0000) | ((v >> 8) & 0xff00) | ((v << 24) & 0xff000000); }

Font::Font(const char* path) {
    f = fopen(path, "rb"); if (!f) return;
    fread(&header, sizeof(OffsetSubtable), 1, f);
    uint16_t numTables = swap16(header.numTables);
    std::vector<TableRecord> tables(numTables);
    uint32_t cmapOffset = 0, headOffset = 0;
    for (int i = 0; i < numTables; i++) {
        fread(&tables[i], sizeof(TableRecord), 1, f);
        uint32_t offset = swap32(tables[i].offset);
        if (strncmp(tables[i].tag, "cmap", 4) == 0) cmapOffset = offset;
        else if (strncmp(tables[i].tag, "head", 4) == 0) headOffset = offset;
        else if (strncmp(tables[i].tag, "loca", 4) == 0) locaoffset = offset;
        else if (strncmp(tables[i].tag, "maxp", 4) == 0) { fseek(f, offset, SEEK_SET); MaxpTable m; fread(&m, sizeof(MaxpTable), 1, f); numGlyphs = swap16(*(uint16_t*)m.numGlyphs); }
        else if (strncmp(tables[i].tag, "glyf", 4) == 0) glyphIndex = offset;
    }
    { fseek(f, headOffset, SEEK_SET); HeadTable h; fread(&h, sizeof(HeadTable), 1, f); unitsPerEm = swap16(*(uint16_t*)h.unitsPerEm); locFormat = swap16(*(uint16_t*)h.indexToLocFormat); fontXMin = (int16_t)swap16(*(uint16_t*)h.xMin); fontYMin = (int16_t)swap16(*(uint16_t*)h.yMin); }
    { fseek(f, cmapOffset, SEEK_SET); uint16_t version, subCount; fread(&version, 2, 1, f); version = swap16(version); fread(&subCount, 2, 1, f); subCount = swap16(subCount);
      for (int i = 0; i < subCount; i++) {
        uint16_t platformID, encodingID; uint32_t subTableOffset;
        fread(&platformID, 2, 1, f); platformID = swap16(platformID);
        fread(&encodingID, 2, 1, f); encodingID = swap16(encodingID);
        fread(&subTableOffset, 4, 1, f); subTableOffset = swap32(subTableOffset);
        long recPos = ftell(f);
        fseek(f, cmapOffset + subTableOffset, SEEK_SET);
        uint16_t format, length, language, segCountX2; fread(&format, 2, 1, f); format = swap16(format);
        fread(&length, 2, 1, f); length = swap16(length); fread(&language, 2, 1, f); language = swap16(language); fread(&segCountX2, 2, 1, f); segCountX2 = swap16(segCountX2);
        uint16_t segCount = segCountX2 / 2; uint16_t sr, es, rs; fread(&sr, 2, 1, f); fread(&es, 2, 1, f); fread(&rs, 2, 1, f);
        if (format != 4) { fseek(f, recPos, SEEK_SET); continue; }
        endCodes.resize(segCount); for (int s = 0; s < segCount; s++) { fread(&endCodes[s], 2, 1, f); endCodes[s] = swap16(endCodes[s]); }
        fseek(f, 2, SEEK_CUR);
        startCodes.resize(segCount); for (int s = 0; s < segCount; s++) { fread(&startCodes[s], 2, 1, f); startCodes[s] = swap16(startCodes[s]); }
        idDeltas.resize(segCount); for (int s = 0; s < segCount; s++) { fread(&idDeltas[s], 2, 1, f); idDeltas[s] = (int16_t)swap16((uint16_t)idDeltas[s]); }
        idRangeOffsets.resize(segCount); for (int s = 0; s < segCount; s++) { fread(&idRangeOffsets[s], 2, 1, f); idRangeOffsets[s] = swap16(idRangeOffsets[s]); }
        long cur = ftell(f), endp = cmapOffset + subTableOffset + length; int rem = (int)(endp - cur);
        if (rem > 0) { int cnt = rem / 2; glyphIdArray.resize(cnt); for (int s = 0; s < cnt; s++) { fread(&glyphIdArray[s], 2, 1, f); glyphIdArray[s] = swap16(glyphIdArray[s]); } }
        break;
      }
    }
}
Font::~Font() { if (f) fclose(f); }
uint16_t Font::getUnitsPerEm() const { return unitsPerEm; }
uint16_t Font::getGlyphIndex(uint16_t unicode) {
    int segCount = (int)endCodes.size(), segment = -1;
    for (int i = 0; i < segCount; i++) if (endCodes[i] >= unicode) { segment = i; break; }
    if (segment == -1 || startCodes[segment] > unicode) return 0;
    if (idRangeOffsets[segment] == 0) return (uint16_t)((unicode + idDeltas[segment]) & 0xFFFF);
    int idx = (idRangeOffsets[segment] / 2) + (unicode - startCodes[segment]) - (segCount - segment);
    if (idx >= 0 && idx < (int)glyphIdArray.size()) { uint16_t gi = glyphIdArray[idx]; if (gi != 0) return (uint16_t)((gi + idDeltas[segment]) & 0xFFFF); }
    return 0;
}
uint32_t Font::getGlyphOffset(uint16_t gi) {
    if (locFormat == 0) { fseek(f, locaoffset + gi * 2, SEEK_SET); uint16_t v = 0; fread(&v, 2, 1, f); return (uint32_t)swap16(v) * 2; }
    else if (locFormat == 1) { fseek(f, locaoffset + gi * 4, SEEK_SET); uint32_t v = 0; fread(&v, 4, 1, f); return swap32(v); }
    return 0;
}
std::vector<uint8_t> Font::downsample(const bool* src, int srcStride, int srcW, int srcH, int dstW, int dstH) {
    std::vector<uint8_t> out; out.resize((size_t)dstW * dstH);
    for (int dy = 0; dy < dstH; dy++) { int sy0 = (int)((int64_t)dy * srcH / dstH), sy1 = (int)((int64_t)(dy + 1) * srcH / dstH); if (sy1 <= sy0) sy1 = sy0 + 1;
      for (int dx = 0; dx < dstW; dx++) { int sx0 = (int)((int64_t)dx * srcW / dstW), sx1 = (int)((int64_t)(dx + 1) * srcW / dstW); if (sx1 <= sx0) sx1 = sx0 + 1; int cnt = 0, tot = 0;
        for (int sy = sy0; sy < sy1; sy++) for (int sx = sx0; sx < sx1; sx++) { tot++; if (src[(size_t)sy * srcStride + sx]) cnt++; }
        out[(size_t)(dstH - 1 - dy) * dstW + dx] = (uint8_t)(tot ? cnt * 255 / tot : 0); } }
    return out;
}
bool Font::drawGlyph(uint32_t glyphoffset, uint32_t* bitmap, uint64_t bw, uint64_t bh) {
    if (!f || !bitmap || bw == 0 || bh == 0) return false;
    GlyfHeader gh; std::vector<int16_t> contours; std::vector<uint8_t> instr, flags; std::vector<int> xs2, ys2;
    fseek(f, glyphIndex + glyphoffset, SEEK_SET); fread(&gh, sizeof(GlyfHeader), 1, f);
    int16_t nC = (int16_t)swap16(*(uint16_t*)gh.numberOfContours);
    int EM = unitsPerEm; if (EM <= 0) return false;   // 공통 정사각 캔버스 = em 단위
    if (nC < 0) return false;
    for (int i = 0; i < nC; i++) { int16_t e; fread(&e, 2, 1, f); contours.push_back((int16_t)swap16((uint16_t)e)); }
    if (contours.empty()) return false;
    uint32_t numPoints = contours.back() + 1; uint16_t il; fread(&il, 2, 1, f); il = swap16(il);
    for (int j = 0; j < il; j++) { uint8_t x; fread(&x, 1, 1, f); instr.push_back(x); }
    for (uint32_t i = 0; i < numPoints; i++) { uint8_t fl; fread(&fl, 1, 1, f); flags.push_back(fl); if (fl & 0x08) { uint8_t r; fread(&r, 1, 1, f); for (int j = 0; j < r; j++) flags.push_back(fl); i += r; } }
    for (uint32_t i = 0; i < numPoints; i++) { int x; if (flags[i] & 0x02) { uint8_t b; fread(&b, 1, 1, f); x = (flags[i] & 0x10) ? b : -b; } else if (!(flags[i] & 0x10)) { int16_t s; fread(&s, 2, 1, f); x = (int16_t)swap16((uint16_t)s); } else x = 0; xs2.push_back(x); }
    for (uint32_t i = 0; i < numPoints; i++) { int y; if (flags[i] & 0x04) { uint8_t b; fread(&b, 1, 1, f); y = (flags[i] & 0x20) ? b : -b; } else if (!(flags[i] & 0x20)) { int16_t s; fread(&s, 2, 1, f); y = (int16_t)swap16((uint16_t)s); } else y = 0; ys2.push_back(y); }
    std::vector<GlyphPoint> rp; rp.push_back({ xs2[0], ys2[0], (flags[0] & 0x01) != 0 });
    for (uint32_t i = 1; i < numPoints; i++) { GlyphPoint l = rp.back(); rp.push_back({ l.x + xs2[i], l.y + ys2[i], (flags[i] & 0x01) != 0 }); }
    std::vector<std::vector<Glyph>> gs;
    for (int i = 0; i < (int)contours.size(); i++) { int start = (i == 0) ? 0 : contours[i - 1] + 1, end = contours[i]; std::vector<Glyph> cg; GlyphPoint cur, cs; int j;
        if (rp[start].onCurve) { cur = rp[start]; j = start + 1; } else if (rp[end].onCurve) { cur = rp[end]; j = start; } else { cur.x = (rp[start].x + rp[end].x) / 2; cur.y = (rp[start].y + rp[end].y) / 2; cur.onCurve = true; j = start; }
        cs = cur;
        while (j <= end) { Glyph sg; sg.p1 = cur; if (rp[j].onCurve) { sg.p2 = rp[j]; sg.p3 = rp[j]; cur = rp[j]; j += 1; } else { sg.p2 = rp[j]; bool hn = (j + 1 <= end); if (hn && rp[j + 1].onCurve) { sg.p3 = rp[j + 1]; cur = rp[j + 1]; j += 2; } else if (hn) { GlyphPoint m; m.x = (rp[j].x + rp[j + 1].x) / 2; m.y = (rp[j].y + rp[j + 1].y) / 2; m.onCurve = true; sg.p3 = m; cur = m; j += 1; } else { sg.p3 = cs; cur = cs; j += 1; } } cg.push_back(sg); }
        if (cur.x != cs.x || cur.y != cs.y) { Glyph sg; sg.p1 = cur; sg.p2 = cs; sg.p3 = cs; cg.push_back(sg); }
        gs.push_back(cg);
    }
    std::vector<std::vector<GlyphPoint>> lines;
    for (int c = 0; c < (int)gs.size(); c++) { std::vector<GlyphPoint> cl;
        for (int s = 0; s < (int)gs[c].size(); s++) { Glyph& sg = gs[c][s]; if (sg.p2.x == sg.p3.x && sg.p2.y == sg.p3.y) cl.push_back(sg.p1);
          else for (int st = 0; st < BEZIER_STEPS; st++) { int T = st, MT = BEZIER_STEPS - st, W = BEZIER_STEPS * BEZIER_STEPS; GlyphPoint p; p.x = (MT * MT * sg.p1.x + 2 * MT * T * sg.p2.x + T * T * sg.p3.x + W / 2) / W; p.y = (MT * MT * sg.p1.y + 2 * MT * T * sg.p2.y + T * T * sg.p3.y + W / 2) / W; p.onCurve = true; cl.push_back(p); } }
        lines.push_back(cl);
    }
    bool* gd = new bool[(size_t)EM * EM]();
    struct Crossing { int x; int w; }; std::vector<Crossing> cr;
    for (int row = 0; row < EM; row++) { int sy2 = 2 * row + 1; cr.clear();   // 2*(픽셀 중심) → 0.5 회피
        for (int c = 0; c < (int)lines.size(); c++) { int n = (int)lines[c].size(); if (n < 2) continue;
            for (int i = 0; i < n; i++) { GlyphPoint& a = lines[c][i]; GlyphPoint& b = lines[c][(i + 1) % n]; int y0 = (a.y - fontYMin) * 2, y1 = (b.y - fontYMin) * 2; if (y0 == y1) continue; if (sy2 < (y0 < y1 ? y0 : y1) || sy2 >= (y0 < y1 ? y1 : y0)) continue; int x = (a.x - fontXMin) + (int)((long)(sy2 - y0) * (b.x - a.x) / (y1 - y0)); cr.push_back({ x, y1 > y0 ? 1 : -1 }); } }
        if (cr.empty()) continue; std::sort(cr.begin(), cr.end(), [](const Crossing& l, const Crossing& r) { return l.x < r.x; });
        int w = 0, ss = 0; for (int i = 0; i < (int)cr.size(); i++) { int bf = w; w += cr[i].w; if (bf == 0 && w != 0) ss = cr[i].x; else if (bf != 0 && w == 0) { int xs = ss, xe = cr[i].x - 1; if (xs < 0) xs = 0; if (xe >= EM) xe = EM - 1; if (row < 0 || row >= EM) continue; for (int x = xs; x <= xe; x++) gd[(size_t)row * EM + x] = true; } }
    }
    std::vector<uint8_t> gray = downsample(gd, EM, EM, EM, (int)bw, (int)bh); delete[] gd;
    for (size_t i = 0; i < (size_t)bw * bh; i++) { uint8_t v = 255 - gray[i]; bitmap[i] = ((uint32_t)v << 16) | ((uint32_t)v << 8) | v; }
    return true;
}

// ── UTF-8 유틸 ──
// 한 코드포인트 디코드. 잘린/불량 시퀀스는 1바이트만 소비(널 넘어 읽지 않도록 단계별 검사).
static uint32_t utf8_next(const char* p, int* len) {
    unsigned char c0 = (unsigned char)p[0];
    if (c0 < 0x80) { *len = 1; return c0; }
    if ((c0 & 0xE0) == 0xC0) {
        unsigned char c1 = (unsigned char)p[1]; if ((c1 & 0xC0) != 0x80) { *len = 1; return c0; }
        *len = 2; return ((uint32_t)(c0 & 0x1F) << 6) | (c1 & 0x3F);
    }
    if ((c0 & 0xF0) == 0xE0) {
        unsigned char c1 = (unsigned char)p[1]; if ((c1 & 0xC0) != 0x80) { *len = 1; return c0; }
        unsigned char c2 = (unsigned char)p[2]; if ((c2 & 0xC0) != 0x80) { *len = 1; return c0; }
        *len = 3; return ((uint32_t)(c0 & 0x0F) << 12) | ((uint32_t)(c1 & 0x3F) << 6) | (c2 & 0x3F);
    }
    if ((c0 & 0xF8) == 0xF0) {
        unsigned char c1 = (unsigned char)p[1]; if ((c1 & 0xC0) != 0x80) { *len = 1; return c0; }
        unsigned char c2 = (unsigned char)p[2]; if ((c2 & 0xC0) != 0x80) { *len = 1; return c0; }
        unsigned char c3 = (unsigned char)p[3]; if ((c3 & 0xC0) != 0x80) { *len = 1; return c0; }
        *len = 4; return ((uint32_t)(c0 & 0x07) << 18) | ((uint32_t)(c1 & 0x3F) << 12) | ((uint32_t)(c2 & 0x3F) << 6) | (c3 & 0x3F);
    }
    *len = 1; return c0;
}
// 코드포인트 → UTF-8, 바이트 수 반환
static int utf8_encode(uint32_t cp, char* out) {
    if (cp < 0x80) { out[0] = (char)cp; return 1; }
    if (cp < 0x800) { out[0] = (char)(0xC0 | (cp >> 6)); out[1] = (char)(0x80 | (cp & 0x3F)); return 2; }
    if (cp < 0x10000) { out[0] = (char)(0xE0 | (cp >> 12)); out[1] = (char)(0x80 | ((cp >> 6) & 0x3F)); out[2] = (char)(0x80 | (cp & 0x3F)); return 3; }
    out[0] = (char)(0xF0 | (cp >> 18)); out[1] = (char)(0x80 | ((cp >> 12) & 0x3F)); out[2] = (char)(0x80 | ((cp >> 6) & 0x3F)); out[3] = (char)(0x80 | (cp & 0x3F)); return 4;
}
// 더블폭(전각: 한글/CJK 등) 여부
static bool cp_is_wide(uint32_t cp) {
    return (cp >= 0x1100 && cp <= 0x115F) ||   // 한글 자모
           (cp >= 0x2E80 && cp <= 0xA4CF) ||   // CJK 부수 ~ 한자
           (cp >= 0xAC00 && cp <= 0xD7A3) ||   // 한글 음절
           (cp >= 0xF900 && cp <= 0xFAFF) ||   // CJK 호환
           (cp >= 0xFF00 && cp <= 0xFF60) ||   // 전각 형태
           (cp >= 0xFFE0 && cp <= 0xFFE6);
}

void Font::draw(const char* text, uint32_t* buf, uint64_t w, uint64_t h,
                uint64_t px, uint64_t advance, int64_t x0, int64_t y0, uint32_t color) {
    if (!f || !buf || !text) return;
    if (px == 0) px = h;
    if (px == 0) return;
    if (advance == 0) advance = px;
    uint8_t cr = (color >> 16) & 0xFF, cg = (color >> 8) & 0xFF, cb = color & 0xFF;
    std::vector<uint32_t> cell; cell.resize(px * px);
    int64_t penX = x0, penY = y0;
    for (const char* p = text; *p; ) {
        if (*p == '\n') { penX = x0; penY += (int64_t)px; ++p; continue; }
        int clen; uint32_t cp = utf8_next(p, &clen); p += clen;
        for (uint64_t i = 0; i < px * px; i++) cell[i] = 0x00FFFFFF;   // 흰 배경으로 초기화
        uint16_t gi = getGlyphIndex((uint16_t)cp);
        if (drawGlyph(getGlyphOffset(gi), &cell[0], px, px)) {
            for (uint64_t gy = 0; gy < px; gy++) { int64_t by = penY + (int64_t)gy; if (by < 0 || by >= (int64_t)h) continue;
                for (uint64_t gx = 0; gx < px; gx++) { int64_t bx = penX + (int64_t)gx; if (bx < 0 || bx >= (int64_t)w) continue;
                    uint8_t cov = 255 - (uint8_t)(cell[gy * px + gx] & 0xFF);   // drawGlyph는 검정글자/흰배경 → 커버리지 복원
                    if (cov == 0) continue;
                    uint32_t dst = buf[by * (int64_t)w + bx];
                    uint8_t dr = (dst >> 16) & 0xFF, dg = (dst >> 8) & 0xFF, db = dst & 0xFF;
                    uint8_t rr = (uint8_t)((cr * cov + dr * (255 - cov)) / 255);
                    uint8_t rg = (uint8_t)((cg * cov + dg * (255 - cov)) / 255);
                    uint8_t rb = (uint8_t)((cb * cov + db * (255 - cov)) / 255);
                    buf[by * (int64_t)w + bx] = ((uint32_t)rr << 16) | ((uint32_t)rg << 8) | rb;
                }
            }
        }
        penX += (int64_t)advance;
    }
}
