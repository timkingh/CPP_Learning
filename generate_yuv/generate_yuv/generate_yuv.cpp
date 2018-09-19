#include <iostream>
#include <fstream>
#include <string.h>  /* needed by sscanf */
#include <string>
#include <vector>
#include "getopt.hpp"
#include "common.h"
#include "generate_yuv.h"

using namespace std;

#ifdef _WIN32
#define SSCANF sscanf_s
#else
#define SSCANF sscanf
#endif

typedef struct BlkInfo {
    uint32_t y_left;
    uint32_t y_top;
    uint32_t uv_left;
    uint32_t uv_top;
    uint32_t mb_size;
    uint32_t y_pixel;
    uint32_t u_pixel;
    uint32_t v_pixel;
} BlkInfo;

static void fill_block(BufInfo *buf_info, BlkInfo *blk_info)
{
    uint32_t col, row;
    uint32_t x0, y0, mb_size;
    uint32_t width = buf_info->width;
    uint32_t height = buf_info->height;
    char *buf_y = buf_info->buf;
    char *buf_u = buf_info->buf + width * height;
    char *buf_v = buf_u + width * height / 4;
    x0 = blk_info->y_left;
    y0 = blk_info->y_top;
    mb_size = blk_info->mb_size;

    /* Y */
    char *buf = buf_y + x0 + y0 * width;
    for (row = 0; row < mb_size; row++) {
        for (col = 0; col < mb_size; col++) {
            buf[row * width + col] = blk_info->y_pixel;
        }
    }

    uint32_t chroma_width = width / 2;
    x0 = blk_info->uv_left;
    y0 = blk_info->uv_top;

    /* U */
    buf = buf_u + x0 + y0 * chroma_width;
    for (row = 0; row < mb_size; row++) {
        for (col = 0; col < mb_size; col++) {
            buf[row * chroma_width + col] = blk_info->u_pixel;
        }
    }

    /* V */
    buf = buf_v + x0 + y0 * chroma_width;
    for (row = 0; row < mb_size; row++) {
        for (col = 0; col < mb_size; col++) {
            buf[row * chroma_width + col] = blk_info->v_pixel;
        }
    }
}


static void fill_blocks(GenCtx *ctx, char *buf)
{
    BlkInfo blk_info;
    blk_info.y_left = 0;
    blk_info.y_top = 0;
    blk_info.uv_left = 0;
    blk_info.uv_top = 0;
    blk_info.mb_size = 4;
    blk_info.y_pixel = 0;
    blk_info.u_pixel = 0;
    blk_info.v_pixel = 0;

    BufInfo buf_info;
    buf_info.buf = buf;
    buf_info.width = ctx->width;
    buf_info.height = ctx->height;

    fill_block(&buf_info, &blk_info);
}

static void generate_yuv(GenCtx *ctx)
{
    uint32_t luma_size = ctx->width * ctx->height;
    uint32_t chroma_size = luma_size / 2;
    uint32_t frame_size = luma_size + chroma_size;
    uint32_t frame_cnt, region_num, region_idx;
    char *buf = new char[frame_size];

    do {
        ctx->ifs->read(buf, frame_size);

        fill_blocks(ctx, buf);

        ctx->ofs->write(buf, frame_size);

        ctx->frame_read++;
    } while (ctx->frame_read < ctx->frames);

    delete [] buf;
}

int main(int argc, char **argv)
{
    GenCtx proc_ctx;
    GenCtx *ctx = &proc_ctx;
    memset(ctx, 0, sizeof(GenCtx));

    cout << "----------Test-------------" << endl;
    bool help = getarg(false, "-H", "--help", "-?");
    string in_file = getarg("F:\\rkvenc_verify\\input_yuv\\Bus_352x288_25.yuv", "-i", "--input");
    string out_file = getarg("F:\\rkvenc_verify\\input_yuv\\Bus_352x288_25_out.yuv", "-o", "--output");
    ctx->width = getarg(352, "-w", "--width");
    ctx->height = getarg(288, "-h", "--height");
    ctx->frames = getarg(5, "-f", "--frames");

    if (help) {
        cout << "Usage:" << endl
             << "./yuv_process -i=3903_720x576.yuv -o=3903_720x576_hi_rk.yuv "
             << "-c=3903.md -s=3903_720x576_150.sad -f=2"
             << endl;
        return 0;
    }

    cout << "input: " << in_file << endl
         << "output: " << out_file << endl;
    ctx->ifs = new ifstream(in_file.c_str(), ios::binary | ios::in);

    ofstream ofs;
    ctx->ofs = &ofs;
    ofs.open(out_file.c_str(), ios::binary | ios::out);

    generate_yuv(ctx);

    if (ctx->ifs && ctx->ifs != &cin)
        delete ctx->ifs;
    ofs.close();

    cout << "----------End!-------------" << endl;
    //string str;
    //cin >> str;
    return 0;
}
