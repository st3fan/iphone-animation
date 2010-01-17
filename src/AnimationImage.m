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

#import "AnimationImage.h"

@implementation AnimationImage

@synthesize data = data_;

+ (id) animationImageWithImageHeader: (const AnimationContainerImageHeader*) header data: (NSData*) data
{
	return [[[self alloc] initWithImageHeader: header data: data] autorelease];
}

- (id) initWithImageHeader: (const AnimationContainerImageHeader*) header data: (NSData*) data;
{
	if ((self = [super init]) != nil) {
		header_ = header;
		data_ = [data retain];
	}
	return self;
}

- (void) dealloc
{
	[data_ release];
	[super dealloc];
}

#pragma mark -

- (NSUInteger) width
{
	return header_->width;
}

- (NSUInteger) height
{
	return header_->height;
}

- (NSUInteger) xoffset
{
	return header_->xoffset;
}

- (NSUInteger) yoffset
{
	return header_->yoffset;
}

- (AnimationContainerImageFormat) format
{
	return header_->format;
}

@end