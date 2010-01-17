/*
 * (C) Copyright 2010, Stefan Arentz, Arentz Consulting Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// rle.cc - compress a collection of images to an animation container
//
//  usage: raw destination.animation width height files*
//

#include <ApplicationServices/ApplicationServices.h>
#include "../src/AnimationCommon.h"

#if 0
void GetCropBox(uint32_t* buffer, int width, int height, int& top, int& left, int& bottom, int& right)
{
    top = left = bottom = right = -1;

    for (int y = 0, done = 0; y < height && done == 0; y++) {
        for (int x = 0; x < width && done == 0; x++) {
            if (buffer[(y * width) + x] != 0) {
                done = 1;
                top = y;
            }
        }
    }
    
    for (int x = 0, done = 0; x < width && done == 0; x++) {
        for (int y = 0; y < height && done == 0; y++) {
            if (buffer[(y * width) + x] != 0) {
                done = 1;
                left = x;
            }
        }
    }

    for (int y = height-1, done = 0; y >= 0 && done == 0; y--) {
        for (int x = 0; x < width && done == 0; x++) {
            if (buffer[(y * width) + x] != 0) {
                done = 1;
                bottom = y;
            }
        }
    }
    
    for (int x = width - 1, done = 0; x >= 0 && done == 0; x--) {
        for (int y = 0; y < height && done == 0; y++) {
            if (buffer[(y * width) + x] != 0) {
                done = 1;
                right = x;
            }
        }
    }
}
#endif

static inline unsigned int RLEWriteRun(unsigned char*& dst, unsigned short n, unsigned char c)
{
    //printf("  Writing a run of %d x %x\n", n, c);

    if (n <= 127) {
        //printf("RLEWriteRun.1 Writing 0 = %x\n", n);
        //printf("RLEWriteRun.1 Writing c = %x\n", c);
        *dst++ = n;
        *dst++ = c;
        return 2;
    } else if (n <= 32767) {
        //printf("RLEWriteRun.2 n = %d\n", n);
        //printf("RLEWriteRun.2 Writing 0 = %x\n", ((n & 0xff00) >> 8) | 0x80);
        //printf("RLEWriteRun.2 Writing 1 = %x\n", ((n & 0x00ff) >> 0));
        //printf("RLEWriteRun.2 Writing c = %x\n", c);
        *dst++ = ((n & 0xff00) >> 8) | 0x80;
        *dst++ = ((n & 0x00ff) >> 0);
        *dst++ = c;
        return 3;
    } else {
        fprintf(stderr, "Fail\n");
    }

    return 0;
}

unsigned int RLECompressRGBA(unsigned char* dst, unsigned char* src, unsigned int count)
{
    //printf("Compressing RGBA pixel data with %d pixels\n", count);

    unsigned int compressedLength = 0;

    for (int channel = 0; channel < 4; channel++)
    {
        //printf("Compressing channel %d\n", channel);

        unsigned char* p = src + channel;
        unsigned char c;
        unsigned short n = 0;

        for (int i = 0; i < count; i++, p += 4)
        {
            //printf("   Looking at %.2x\n", *p);

            if (n == 0) {
                //fprintf(stderr, "      Starting a new run: n = 1 c = %d\n", c);
                c = *p;
                n = 1;
            } else {
                if (*p == c) {
                    n++;
                    //fprintf(stderr, "      More of the same: n = %d c = %d\n", n, c);
                    if (n == 32767) {
                        compressedLength += RLEWriteRun(dst, n, c);
                        n = 0;
                    }
                } else {
                    //fprintf(stderr, "      Run changed, writing run: n = %d c = %d\n", n, c);
                    compressedLength += RLEWriteRun(dst, n, c);
                    c = *p;
                    n = 1;
                }
            }
        }
        
        if (n != 0) {
            compressedLength += RLEWriteRun(dst, n, c);
        }
    }

    return compressedLength;
}

unsigned int RLEUncompressRGBA(unsigned char* dst, unsigned char* src, unsigned int count)
{
    unsigned int length = 0;

    for (int channel = 0; channel < 4; channel++)
    {
        unsigned char* p = dst + channel;
    
        unsigned int t = count;
        while (t != 0)
        {
            unsigned short n = *src++;
            if (n & 0x80) {
                n = (n & 0x7f) << 8;
                n |= *src++;
            }
            
            unsigned char c = *src++;
            
            for (int i = 0; i < n; i++) {
                *p = c;
                p += 4;
                length++;
            }
            
            t -= n;
        }
    }
    
    return length;
}

int main(int argc, char** argv)
{
    // Parse command line arguments

    int width = atoi(argv[2]);
    int height = atoi(argv[3]);

    // Create the animation container

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1)
    {
        // Create a buffer for the images

        uint32_t* buffer = (uint32_t*) calloc(width * height, sizeof(uint32_t));

        // Write the container header

        AnimationContainerGlobalHeader containerHeader;
        containerHeader.magic = 'anim';
        containerHeader.version = 1;
        containerHeader.width = width;
        containerHeader.height = height;
        containerHeader.frameRate = 12;
        containerHeader.frameCount = argc - 4;

        write(fd, &containerHeader, sizeof(containerHeader));

        // Write all the images

        for (int i = 4; i < argc; i++)
        {
            char* path = argv[i];
            printf("Processing %s\n", path);
            
            // Uncompress the image

            memset(buffer, 0x00, width * height * sizeof(uint32_t));
            
            CGDataProviderRef provider = CGDataProviderCreateWithFilename(path);
            if (provider != NULL)
            {
                CGImageRef image = CGImageCreateWithPNGDataProvider(provider, NULL, false, kCGRenderingIntentDefault);
                if (image != NULL)
                {
                    size_t imageWidth = CGImageGetWidth(image);
                    size_t imageHeight = CGImageGetHeight(image);

                    if (imageWidth != width || imageHeight != height) {
                        fprintf(stderr, "Image %s is not of size %dx%d\n", path, width, height);
                        exit(1);
                    }
                    
                    // Create a new bitmap
                    
                    CGContextRef context = CGBitmapContextCreate(
                        (void*) buffer,
                        width,
                        height,
                        8,                             // Bits per component
                        width * 4,                     // Bytes per row
                        CGImageGetColorSpace(image),
                        kCGImageAlphaPremultipliedLast // RRRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
                    );
                        
                    if (context != NULL)
                    {
                        CGContextDrawImage(context, CGRectMake(0.0f, 0.0f, width, height), image);
                        CGContextRelease(context);
                    }
                    
                    CGImageRelease(image);
                }
                
                CFRelease(provider);
            }

            // Compress the buffer
            
            unsigned char* compressedBuffer = (unsigned char*) malloc(width * height * sizeof(uint32_t) * 3);
            if (compressedBuffer == NULL) {
                printf("Can't allocate memory\n");
                exit(1);
            }

            uint32_t compressedLength = RLECompressRGBA(compressedBuffer, (unsigned char*) buffer, width * height);
            printf("Compressed length = %d\n", compressedLength);

#if 1
            // Sanity check

            unsigned char* uncompressedBuffer = (unsigned char*) malloc(width * height * sizeof(uint32_t));
            if (uncompressedBuffer == NULL) {
                printf("Can't allocate memory\n");
                exit(1);
            }

            uint32_t uncompressedLength = RLEUncompressRGBA(uncompressedBuffer, (unsigned char*) compressedBuffer,
                width * height);
            printf("Uncompressed length = %d\n", uncompressedLength);
            
            if (uncompressedLength != (width * height * sizeof(uint32_t))) {
                printf("Decompression fail. Data length not equal to original.\n");
                exit(1);
            }

            if (memcmp(buffer, uncompressedBuffer, width * height * sizeof(uint32_t)) != 0) {
                printf("Decompression fail. Buffers are not equal!\n");
                exit(1);
            }
#endif
       
            // Write the image header
            
            AnimationContainerImageHeader imageHeader;
            imageHeader.width = width;
            imageHeader.height = height;
            imageHeader.xoffset = 0;
            imageHeader.yoffset = 0;
            imageHeader.format = AnimationContainerImageFormatRunLengthCompressedPixels;
            imageHeader.dataLength = compressedLength;

            write(fd, &imageHeader, sizeof(imageHeader));

            // Write the compressed image data

            write(fd, compressedBuffer, compressedLength);
            free(compressedBuffer);
            
            // Write the padding if needed
            
            if ((compressedLength % 4) > 0) {
                write(fd, "\0\0\0", 4 - (compressedLength % 4));
            }
        }

        close(fd);
    }
}
