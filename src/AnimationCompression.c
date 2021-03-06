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
 
#include "AnimationCompression.h"

void AnimationDecompressRunLengthEncodedPixels(uint32_t* dst, uint32_t* src, uint32_t count)
{
	while (count != 0)
	{
		uint32_t n = *src++;
		uint32_t c = *src++;
		
		for (int i = 0; i < n; i++) {
			*dst++ = c;
			count--;
		}
	}
}

uint32_t AnimationCompressRunLengthEncodedPixels(uint32_t* dst, uint32_t* src, unsigned int count)
{
    unsigned int compressedLength = 0;

    uint32_t c;
    uint32_t n = 0;
    
    for (int i = 0; i < count; i++, src++)
    {
        if (n == 0) {
            c = *src;
            n = 1;
        } else {
            if (*src == c) {
                n++;
            } else {
				*dst++ = n; *dst++ = c; compressedLength += 2;
                c = *src;
                n = 1;
            }
        }
    }
    
    if (n != 0) {
		*dst++ = n; *dst++ = c; compressedLength += 2;
    }
    
    return compressedLength * 4;
}