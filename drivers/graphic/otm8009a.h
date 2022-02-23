/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        otm8009a.h
 *
 * @brief       This file provides otm8009a driver functions.
 *
 * @details     otm8009a
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef OTM8009_H_
#define OTM8009_H_
 
/* Width and Height in Portrait mode */
#define OTM8009A_480X800_WIDTH  ((uint16_t)480) /* LCD PIXEL WIDTH   */
#define OTM8009A_480X800_HEIGHT ((uint16_t)800) /* LCD PIXEL HEIGHT  */

/* Width and Height in Landscape mode */
#define OTM8009A_800X480_WIDTH  ((uint16_t)800) /* LCD PIXEL WIDTH   */
#define OTM8009A_800X480_HEIGHT ((uint16_t)480) /* LCD PIXEL HEIGHT  */

#define OTM8009A_480X800_HSYNC ((uint16_t)2)  /* Horizontal synchronization */
#define OTM8009A_480X800_HBP   ((uint16_t)34) /* Horizontal back porch      */
#define OTM8009A_480X800_HFP   ((uint16_t)34) /* Horizontal front porch     */
#define OTM8009A_480X800_VSYNC ((uint16_t)1)  /* Vertical synchronization   */
#define OTM8009A_480X800_VBP   ((uint16_t)15) /* Vertical back porch        */
#define OTM8009A_480X800_VFP   ((uint16_t)16) /* Vertical front porch       */

#define OTM8009A_800X480_HSYNC OTM8009A_480X800_VSYNC /* Horizontal synchronization */
#define OTM8009A_800X480_HBP   OTM8009A_480X800_VBP   /* Horizontal back porch      */
#define OTM8009A_800X480_HFP   OTM8009A_480X800_VFP   /* Horizontal front porch     */
#define OTM8009A_800X480_VSYNC OTM8009A_480X800_HSYNC /* Vertical synchronization   */
#define OTM8009A_800X480_VBP   OTM8009A_480X800_HBP   /* Vertical back porch        */
#define OTM8009A_800X480_VFP   OTM8009A_480X800_HFP   /* Vertical front porch       */

#endif // OTM8009_H_
