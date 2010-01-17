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
// rle.cc - compress a collection of images to an animation container. images are
//     compressed using a simple run-length encoding.
//
//   usage: raw destination.animation width height files*
//

#include <ApplicationServices/ApplicationServices.h>
#include "../src/AnimationCommon.h"
#include "../src/AnimationCompression.h"

int main(int argc, char** argv)
{
    // Parse command line arguments

    int width = atoi(argv[2]);
    int height = atoi(argv[3]);

    // Create the animation container

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        fprintf(stderr, "Cannot create %s: %s\n", argv[1], strerror(errno));
        exit(1);
    }

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

        uint32_t compressedLength = AnimationCompressRunLengthEncodedPixels(compressedBuffer, buffer, width * height);
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

        AnimationDecompressRunLengthEncodedPixels(uncompressedBuffer, compressedBuffer, width * height);
        
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
    
    // Close the animation container
    
    close(fd);
}
