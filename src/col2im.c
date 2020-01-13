#include <stdio.h>
#include <math.h>
void col2im_add_pixel(float *im, int height, int width, int channels,
                        int row, int col, int channel, int pad, float val)
{
    row -= pad;
    col -= pad;

    if (row < 0 || col < 0 || row >= height || col >= width)
        return;
    im[col + width*(row + height*channel)] += val;
}
//This one might be too, can't remember.
void col2im_cpu(float* data_col,
         int channels,  int height,  int width,
         int ksize,  int stride, int pad, float* data_im) 
{
    int c, h, w;
    int height_col = (height + 2 * pad - ksize) / stride + 1;
    int width_col = (width + 2 * pad - ksize) / stride + 1;

    // 一个卷积核覆盖的像素的个数, 简单表示为 k*k*c, 每个像素其实最后会和原图像的 oh*ow 个像素发生关系,
    // 因为卷积核会纵向移动 oh, 横向移动 ow, 那么与卷积核一个像素有关的 oh*ow 个原图像的像素组成一个特征图
    // 那么就可以排列成 k*k*c 个 oh*ow 的特征图, 表示成 (oh*ow, k*k*c) 的二维数组 或者 oh*ow*k*k*c 的一维数组
    int channels_col = channels * ksize * ksize;
    for (c = 0; c < channels_col; ++c) {
        int w_offset = c % ksize;
        int h_offset = (c / ksize) % ksize;
        int c_im = c / ksize / ksize;
        // 卷积核移动的次数, 纵向 height_col, 横向 width_col, 寻找原图像中与当前卷积核像素有关的 height_col * width_col 个像素
        for (h = 0; h < height_col; ++h) {
            for (w = 0; w < width_col; ++w) {
                // 卷积核中某个像素对应于原图像的像素的位置, 两者的深度位置是一样的, 都是 c_im, 只是长度宽度的位置不同
                int im_row = h_offset + h * stride;
                int im_col = w_offset + w * stride;
                int col_index = (c * height_col + h) * width_col + w;
                // 这个值是卷积核的某个像素在某个移动位置产生的 delta, 作用于该位置对应的原图像的像素
                double val = data_col[col_index];
                // 原图像的像素的 delta 可能受多个卷积核像素 delta 的影响, 所以此处采用叠加
                // 多个卷积核像素是指, 同一个卷积核的不同像素在不同 offset 时, 可能会重合
                // 比如 3*3 的卷积核, 在 h=w=0 时, 卷积核的第 (0,1) 像素作用于原图像 (0,1) 像素
                // 当 h=0,w=1 时, 卷积核第 (0,0) 像素作用于原图像的 (0,1) 像素
                // 因此这两次卷积核的 delta 应叠加于原图像的 (0,1) 像素
                col2im_add_pixel(data_im, height, width, channels, im_row, im_col, c_im, pad, val);
            }
        }
    }
}

