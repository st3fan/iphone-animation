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

inline uint32_t RLEWriteRun(uint32_t*& dst, uint32_t n, uint32_t c)
{
    //printf("         Writing a run of %d x %x\n", n, c);

    *dst++ = n;
    *dst++ = c;
    
    return 2;
}

unsigned int RLECompressPixels(uint32_t* dst, uint32_t* src, unsigned int count)
{
    //printf("Compressing RGBA pixel data with %d pixels\n", count);
    
    unsigned int compressedLength = 0;

    uint32_t* p = src;
    uint32_t c;
    uint32_t n = 0;
    
    for (int i = 0; i < count; i++, p++)
    {
        //printf("   Looking at 0x%.8x\n", *p);
        
        if (n == 0) {
            //fprintf(stderr, "      Starting a new run: n = 1 c = 0x%.8x\n", c);
            c = *p;
            n = 1;
        } else {
            if (*p == c) {
                n++;
                //fprintf(stderr, "      More of the same: n = %d c = 0x%.8x\n", n, c);
            } else {
                //fprintf(stderr, "      Run changed, writing run: n = %d c = 0x%.8x\n", n, c);
                compressedLength += RLEWriteRun(dst, n, c);
                c = *p;
                n = 1;
            }
        }
    }
    
    if (n != 0) {
        compressedLength += RLEWriteRun(dst, n, c);
    }
    
    return compressedLength * 4;
}

uint32_t RLEUncompressPixels(uint32_t* dst, uint32_t* src, uint32_t count)
{
    uint32_t length = 0;
    
    while (count != 0)
    {
        uint32_t n = *src++;
        uint32_t c = *src++;
        
        //printf("Uncompressing a run of %d x %.8x\n", n, c);

        for (int i = 0; i < n; i++) {
            *dst++ = c;
            count--;
            length += 4;
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
            
            uint32_t* compressedBuffer = (uint32_t*) malloc(width * height * sizeof(uint32_t) * 2);
            if (compressedBuffer == NULL) {
                printf("Can't allocate memory\n");
                exit(1);
            }

            uint32_t compressedLength = RLECompressPixels(compressedBuffer, buffer, width * height);
            printf("Compressed length = %d\n", compressedLength);

#if 0
            printf("Compressed buffer:\n");
            for (int i = 0; i < compressedLength / sizeof(uint32_t); i++) {
                printf(" Buffer[%.4d] = 0x%.8x\n", i, compressedBuffer[i]);
            }
#endif

#if 1
            // Sanity check

            uint32_t* uncompressedBuffer = (uint32_t*) malloc(width * height * sizeof(uint32_t));
            if (uncompressedBuffer == NULL) {
                printf("Can't allocate memory\n");
                exit(1);
            }

            uint32_t uncompressedLength = RLEUncompressPixels(uncompressedBuffer, compressedBuffer, width * height);
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
