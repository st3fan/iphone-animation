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

#import "AnimationView.h"

@implementation AnimationView

@synthesize animation = animation_;

#pragma mark -

- (id) initWithCoder: (NSCoder*) coder
{
	if ((self = [super initWithCoder: coder]) != nil)
	{
 		imageView_ = [[[UIImageView alloc] initWithFrame: CGRectMake(0, 0, self.frame.size.width, self.frame.size.height)] autorelease];
		[self addSubview: imageView_];

		framesPerSecond_ = 12.0;
		frameBuffer_ = [[AnimationFrameBuffer alloc] initWithWidth: self.frame.size.width height: self.frame.size.height];
	}
	return self;
}

- (void) dealloc
{
	[super dealloc];
}

#pragma mark -

- (void) displayNextFrame: (NSTimer*) timer
{
	[animation_ drawFrame: frame_ intoFrameBuffer: frameBuffer_];

	frame_++;
	if (frame_ == animation_.frameCount) {
		frame_ = 0;
	}

	CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, (const void*) frameBuffer_.pixels, self.frame.size.width*self.frame.size.height*4, NULL);
	if (provider != NULL)
	{
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		if (colorSpace != NULL)
		{
			CGImageRef image = CGImageCreate(self.frame.size.width, self.frame.size.height, 8, 32, 4 * self.frame.size.width, colorSpace, kCGImageAlphaNoneSkipLast, provider, NULL, NO, kCGRenderingIntentDefault);
			if (image != NULL)
			{
				imageView_.image = [UIImage imageWithCGImage: image];		
				CGImageRelease(image);
			}
			
			CGColorSpaceRelease(colorSpace);
		}

		CGDataProviderRelease(provider);
	}
}

#pragma mark -

- (void) start
{
	if (timer_ == nil) {
		timer_ = [[NSTimer scheduledTimerWithTimeInterval: 1.0 / framesPerSecond_ target: self
			selector: @selector(displayNextFrame:) userInfo: nil repeats: YES] retain];
	}
}

- (void) stop
{
	if (timer_ != nil) {
		[timer_ invalidate];
		[timer_ release];
		timer_ = nil;
	}
}

@end