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

#import "Animation.h"
#import "AnimationImage.h"

void RLEUncompressRGBA(unsigned char* dst, unsigned char* src, unsigned int count)
{
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
            }
            
            t -= n;
        }
    }
}

@implementation Animation

+ (id) animationNamed: (NSString*) name
{
	Animation* animation = nil;

	NSString* path = [[NSBundle mainBundle] pathForResource: name ofType: @"animation"];
	if (path != nil) {
		animation = [[[self alloc] initWithContentsOfFile: path] autorelease];
	}
	
	return animation;
}

- (id) initWithContentsOfFile: (NSString*) path
{
	if ((self = [super init]) != nil)
	{
		data_ = [[NSData dataWithContentsOfFile: path] retain];
		if (data_ != nil)
		{
			images_ = [NSMutableArray new];
			
			globalHeader_  = (AnimationContainerGlobalHeader*) [data_ bytes];
			const void* p = [data_ bytes] + sizeof(AnimationContainerGlobalHeader);
			
			for (NSUInteger i = 0; i < globalHeader_->frameCount; i++)
			{
				const AnimationContainerImageHeader* imageHeader = (AnimationContainerImageHeader*) p;
			
				AnimationImage* image = nil;

				// TODO: This needs to be delegated to AnimationImage subclasses

				NSData* data = [NSData dataWithBytesNoCopy: (void*) p + sizeof(AnimationContainerImageHeader) length: imageHeader->dataLength freeWhenDone: NO];
				image = [AnimationImage animationImageWithImageHeader: imageHeader data: data];

#if 0
				switch (imageHeader->format)
				{
					case AnimationContainerImageFormatPNG: {
						break;
					}
					case AnimationContainerImageFormatUncompressedPixels: {
						NSLog(@"Format not supported yet");
						exit(1);
						break;
					}
					case AnimationContainerImageFormatRunLengthCompressedPixels: {
						NSLog(@"Format not supported yet");
						exit(1);
						break;
					}
				}
#endif
				
				[images_ addObject: image];
				
				p += sizeof(AnimationContainerImageHeader) + imageHeader->dataLength;
				if ((imageHeader->dataLength % 4) != 0) {
					p += 4 - (imageHeader->dataLength % 4);
				}
			}
		}
		
	}
	
	return self;
}

- (void) dealloc
{
	[images_ release];
	[super dealloc];
}

#pragma mark -

- (void) drawFrame: (NSUInteger) frame intoFrameBuffer: (AnimationFrameBuffer*) buffer
{
	// TODO: All this drawing work should be delegated to AnimationImage subclasses
	
	AnimationImage* animationImage = [images_ objectAtIndex: frame];
	if (animationImage != nil)
	{
		switch (animationImage.format)
		{
			case AnimationContainerImageFormatPNG:
			{
				UIImage* image = [UIImage imageWithData: animationImage.data];
			
				CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
				if (colorSpace != NULL)
				{
					CGContextRef context = CGBitmapContextCreate(buffer.pixels, buffer.width, buffer.height, 8, buffer.width*4, colorSpace, kCGImageAlphaPremultipliedLast);
					if (context != NULL)
					{
						CGContextDrawImage(context, CGRectMake(0, 0, animationImage.width, animationImage.height), image.CGImage);
						CGContextRelease(context);
					}
					
					CGColorSpaceRelease(colorSpace);
				}
				break;
			}
			
			case AnimationContainerImageFormatUncompressedPixels:
			{
				if (animationImage.width == buffer.width && animationImage.height == buffer.height) {
					uint32_t* src = (uint32_t*) [animationImage.data bytes];
					uint32_t* dst = buffer.pixels;
					for (unsigned int i = 0; i < 480*320; i++) {
						*dst++ = *src++;
					}
					//memcpy(buffer.pixels, [animationImage.data bytes],[animationImage.data length]);
				}
				break;
			}
			
			case AnimationContainerImageFormatRunLengthCompressedPixels:
			{
				if (animationImage.width == buffer.width && animationImage.height == buffer.height) {
					RLEUncompressRGBA((unsigned char*) buffer.pixels, (unsigned char*) [animationImage.data bytes],
						animationImage.width * animationImage.height);
				}
			
				break;
			}
		}
	
	}
}

#pragma mark -

- (NSUInteger) frameCount
{
	return globalHeader_->frameCount;
}

- (NSUInteger) frameRate
{
	return globalHeader_->frameRate;
}

@end