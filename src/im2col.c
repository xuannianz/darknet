#include "im2col.h"
#include <stdio.h>
float im2col_get_pixel(float *im, int height, int width, int channels,
                        int row, int col, int channel, int pad)
{
    row -= pad;
    col -= pad;

    if (row < 0 || col < 0 || row >= height || col >= width)
        return 0;
    // 提取 [row, col, channel] 位置的像素, channel * H * W + row * W + col
    return im[col + width*(row + height*channel)];
}

//From Berkeley Vision's Caffe!
//https://github.com/BVLC/caffe/blob/master/LICENSE
void im2col_cpu(float* data_im,
     int channels,  int height,  int width,
     int ksize,  int stride, int pad, float* data_col) 
{
    int c,h,w;
    int height_col = (height + 2*pad - ksize) / stride + 1;
    int width_col = (width + 2*pad - ksize) / stride + 1;

    // 一个卷积核覆盖的像素的个数, 简单表示为 k*k*c, 每个像素其实最后会和原图像的 oh*ow 个像素发生关系,
    // 因为卷积核会纵向移动 oh, 横向移动 ow, 那么与卷积核一个像素有关的 oh*ow 个原图像的像素组成一个特征图
    // 那么就可以排列成 k*k*c 个 oh*ow 的特征图, 表示成 (oh*ow, k*k*c) 的二维数组 或者 oh*ow*k*k*c 的一维数组
    int channels_col = channels * ksize * ksize;
    for (c = 0; c < channels_col; ++c) {
        // 卷积核中某个像素的位置
        int w_offset = c % ksize;
        int h_offset = (c / ksize) % ksize;
        int c_im = c / ksize / ksize;
        // 卷积核移动的次数, 纵向 height_col, 横向 width_col, 寻找原图像中与当前卷积核像素有关的 height_col * width_col 个像素
        for (h = 0; h < height_col; ++h) {
            for (w = 0; w < width_col; ++w) {
                // 卷积核中某个像素对应于原图像的像素的位置, 两者的深度位置是一样的, 都是 c_im, 只是长度宽度的位置不同
                int im_row = h_offset + h * stride;
                int im_col = w_offset + w * stride;
                // 对原图像的像素重新排列的结果, 位置和卷积核操作后的输出的位置对应
                // 可以想象成是 (height_col,width_col,channels_col) 的三维数组转换成的以为一维数组
                // 一共 channels_col 个特征图, 内存到依次存放每个特征图
                int col_index = (c * height_col + h) * width_col + w;
                data_col[col_index] = im2col_get_pixel(data_im, height, width, channels, im_row, im_col, c_im, pad);
            }
        }
    }
}

