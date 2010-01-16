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
// raw.cc - compress a collection of images to an animation container
//
//  usage: raw destination.animation width height files*
//

#include <ApplicationServices/ApplicationServices.h>
#include "../src/AnimationCommon.h"

int main(int argc, char** argv)
{
    // Parse command line arguments

    int width = atoi(argv[2]);
    int height = atoi(argv[3]);

    // Create the animation container

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC);
    if (fd != -1)
    {
        // Create a buffer for the images

        unsigned char* buffer = (unsigned char*) calloc(1, width * height * 4);

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

            // Write the image header
            
            AnimationContainerImageHeader imageHeader;
            imageHeader.width = width;
            imageHeader.height = height;
            imageHeader.xoffset = 0;
            imageHeader.yoffset = 0;
            imageHeader.format = AnimationContainerImageFormatUncompressedPixels;
            imageHeader.dataLength = width * height * 4;

            write(fd, &imageHeader, sizeof(imageHeader));

            // Uncompress the image

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

            // Write the image data

            write(fd, buffer, width * height * 4);
        }

        close(fd);
    }
}
