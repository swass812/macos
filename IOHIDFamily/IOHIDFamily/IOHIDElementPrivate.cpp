/*
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2012 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <AssertMacros.h>
#include <IOKit/IORegistryEntry.h>
#include <IOKit/IOLib.h>
#include "IOHIDElementPrivate.h"
#include "IOHIDEventQueue.h"
#include "IOHIDReportElementQueue.h"
#include "IOHIDDescriptorParserPrivate.h"
#include "IOHIDPrivateKeys.h"
#include "IOHIDDebug.h"
#include "AppleHIDUsageTables.h"
#include "IOHIDUsageTables.h"
#include "IOHIDFamilyTrace.h"
#include "IOHIDFamilyPrivate.h"
#include "IOHIDElementContainer.h"
#include "IOHIDDevice.h"

#define IsRange() \
            (_usageMin != _usageMax)

#define IsArrayElement(element) \
            ((element->_flags & kHIDDataArrayBit) == kHIDDataArray)

#define IsArrayReportHandler(reportHandler) \
            (reportHandler == _arrayReportHandler)

#define IsArrayElementTheReportHandler(element) \
            (element == element->_arrayReportHandler)
            
#define IsButtonElement(element) \
            (element->_reportBits == 1)
            
#define IsDuplicateElement(element) \
            (element->_duplicateReportHandler)
            
#define IsDuplicateReportHandler(reportHandler) \
            (reportHandler == _duplicateReportHandler)
            
#define GetDuplicateElementCount(element) \
            ((element->_duplicateReportHandler) ? element->_duplicateReportHandler->_reportCount : 0)
            
#define GetArrayItemIndex(sel) \
            (sel - _logicalMin)

#define GetArrayItemSel(index) \
            (index + _logicalMin)

			
OSDefineMetaClassAndAbstractStructors(IOHIDElement, OSCollection)
OSMetaClassDefineReservedUsed(IOHIDElement,  0);
OSMetaClassDefineReservedUsed(IOHIDElement,  1);
OSMetaClassDefineReservedUsed(IOHIDElement,  2);
OSMetaClassDefineReservedUsed(IOHIDElement,  3);
OSMetaClassDefineReservedUsed(IOHIDElement,  4);
OSMetaClassDefineReservedUsed(IOHIDElement,  5);
OSMetaClassDefineReservedUsed(IOHIDElement,  6);
OSMetaClassDefineReservedUsed(IOHIDElement,  7);
OSMetaClassDefineReservedUsed(IOHIDElement,  8);
OSMetaClassDefineReservedUsed(IOHIDElement,  9);
OSMetaClassDefineReservedUnused(IOHIDElement, 10);
OSMetaClassDefineReservedUnused(IOHIDElement, 11);
OSMetaClassDefineReservedUnused(IOHIDElement, 12);
OSMetaClassDefineReservedUnused(IOHIDElement, 13);
OSMetaClassDefineReservedUnused(IOHIDElement, 14);
OSMetaClassDefineReservedUnused(IOHIDElement, 15);
OSMetaClassDefineReservedUnused(IOHIDElement, 16);
OSMetaClassDefineReservedUnused(IOHIDElement, 17);
OSMetaClassDefineReservedUnused(IOHIDElement, 18);
OSMetaClassDefineReservedUnused(IOHIDElement, 19);
OSMetaClassDefineReservedUnused(IOHIDElement, 20);
OSMetaClassDefineReservedUnused(IOHIDElement, 21);
OSMetaClassDefineReservedUnused(IOHIDElement, 22);
OSMetaClassDefineReservedUnused(IOHIDElement, 23);
OSMetaClassDefineReservedUnused(IOHIDElement, 24);
OSMetaClassDefineReservedUnused(IOHIDElement, 25);
OSMetaClassDefineReservedUnused(IOHIDElement, 26);
OSMetaClassDefineReservedUnused(IOHIDElement, 27);
OSMetaClassDefineReservedUnused(IOHIDElement, 28);
OSMetaClassDefineReservedUnused(IOHIDElement, 29);
OSMetaClassDefineReservedUnused(IOHIDElement, 30);
OSMetaClassDefineReservedUnused(IOHIDElement, 31);
	
#define super IOHIDElement
OSDefineMetaClassAndStructors( IOHIDElementPrivate, IOHIDElement )

//---------------------------------------------------------------------------
// 
 
bool IOHIDElementPrivate::init( IOHIDElementContainer * owner, IOHIDElementType type )
{
	if ( ( super::init() != true ) || ( owner == 0 ) )
    {
        return false;
    }

    _owner = owner;
    _type  = type;
    _reportSize = 0;
    _reportCount = 1;
    _rawReportCount = 1;
    _duplicateReportHandler = 0;
    _arrayReportHandler = 0;
    _colArrayReportHandlers = 0;
    _arrayItems = 0;
    _duplicateElements = 0;
    _oldArraySelectors = 0;
    _usagePage = 0;
    _usageMin = _usageMax = 0;
    _isInterruptReportHandler = 0;
    
    return true;
}

//---------------------------------------------------------------------------
// 

IOHIDElementPrivate *
IOHIDElementPrivate::buttonElement( IOHIDElementContainer *     owner,
                             IOHIDElementType  type,
                             HIDButtonCapabilitiesPtr  button,
                             IOHIDElementPrivate *    parent )
{
    IOHIDElementPrivate * element = new IOHIDElementPrivate;

    // Check arguments and call init().

    if ( ( button  == 0 ) ||
         ( element == 0 ) ||
         ( element->init( owner, type ) == false ) )
    {
        if ( element ) element->release();
        return 0;
    }

    // Set HID properties.

    element->_flags          = button->bitField;
    element->_reportStartBit = button->startBit;
    element->_reportID       = button->reportID;
    element->_usagePage      = button->usagePage;
    element->_rangeIndex     = 0;    
    element->_logicalMin     = element->_physicalMin = 0;
    element->_logicalMax     = element->_physicalMax = 1;

    if ( button->isRange )
    {
        element->_usageMin = button->u.range.usageMin;
        element->_usageMax = button->u.range.usageMax;        
    }
    else
    {
        element->_usageMin = button->u.notRange.usage;
        element->_usageMax = button->u.notRange.usage;
    }

    if (IsArrayElement(element))
    {
        // RY: Hack to gain the logical min/max for
        // elements.
        element->_logicalMin    = element->_physicalMin = button->u.notRange.reserved2;
        element->_logicalMax    = element->_physicalMax = button->u.notRange.reserved3;
        
        // RY: Hack to gain the report size and report
        // count for Array type elements.  This works 
        // out because array elements do not make use
        // of the unit and unit exponent.  Plus, this
        // keeps us binary compatible.  Appropriate
        // changes have been made to the HIDParser to
        // support this.
        element->_reportBits	= button->unitExponent;
        element->_reportCount	= button->units;
        
        // RY: Let's set the minimum range for keys that this keyboard supports.
        // This is needed because some keyboard describe support for 101 keys, but
        // in actuality support a far greater amount of keys.  Ex. JIS keyboard on
        // Q41B and Q16B.
        if (button->isRange &&
            ( element->_usagePage == kHIDPage_KeyboardOrKeypad ) && 
            ( element->_usageMax < (kHIDUsage_KeyboardLeftControl - 1) ))
        {
            element->_usageMax = kHIDUsage_KeyboardLeftControl - 1;
        }
    }
    else
    {
        element->_reportBits     = 1;
        element->_units          = button->units;
        element->_unitExponent   = button->unitExponent;
    }
    
    element->_rawReportCount = element->_reportCount;
    element->_currentReportSizeBits = element->_reportBits * element->_reportCount;

    // Register with owner and parent, then spawn sub-elements.
    if ( ( parent && ( parent->addChildElement(element, IsArrayElement(element)) == true ) )
    &&   ( owner->registerElement( element, &element->_cookie ) == true )
    &&   ( element->createSubElements() == true ))
    {
        return element;
    }

    element->release();
    element = 0;

    return element;
}

//---------------------------------------------------------------------------
// 

IOHIDElementPrivate *
IOHIDElementPrivate::valueElement( IOHIDElementContainer *     owner,
                            IOHIDElementType  type,
                            HIDValueCapabilitiesPtr   value,
                            IOHIDElementPrivate *    parent )
{
    IOHIDElementPrivate * element = new IOHIDElementPrivate;

    // Check arguments and call init().

    if ( ( value   == 0 ) ||
         ( element == 0 ) ||
         ( element->init( owner, type ) == false ) )
    {
        if ( element ) element->release();
        return 0;
    }

    // Set HID properties.

    element->_flags          = value->bitField;
    element->_reportBits     = value->bitSize;
    element->_reportCount    = value->reportCount;
    element->_reportStartBit = value->startBit;
    element->_reportID       = value->reportID;
    element->_usagePage      = value->usagePage;
    element->_logicalMin     = value->logicalMin;
    element->_logicalMax     = value->logicalMax;
    element->_physicalMin    = value->physicalMin;
    element->_physicalMax    = value->physicalMax;
    element->_units          = value->units;
    element->_unitExponent   = value->unitExponent;
    element->_rangeIndex     = 0;
    element->_rawReportCount = element->_reportCount;

    if ( value->isRange )
    {
        element->_usageMin = value->u.range.usageMin;
        element->_usageMax = value->u.range.usageMax;

        element->_reportCount = 1;
    }
    else
    {
        element->_usageMin = value->u.notRange.usage;
        element->_usageMax = value->u.notRange.usage;
    }
    
    element->_currentReportSizeBits = element->_reportBits *  element->_reportCount;
    
    // If not array of usages then we should avoid duplicate here and report
    // it as single element
    if (element->_reportCount > 1)
    {
        element->_reportBits *= element->_reportCount;
        element->_reportCount = 1;
    }
  
    if (parent && parent->getUsagePage() == kHIDPage_AppleVendor &&
       (parent->getUsage() == kHIDUsage_AppleVendor_Message ||
        parent->getUsage() == kHIDUsage_AppleVendor_Payload))
    {
        element->_variableSize |= kIOHIDElementVariableSizeElement;
    }
    // Register with owner and parent, then spawn sub-elements.

    if ( ( owner->registerElement( element, &element->_cookie ) == true )
    &&   ( ( parent && ( parent->addChildElement(element, IsArrayElement(element)) == true ) ) ) 
    &&   ( element->createSubElements() == true ))
    {
        return element;
    }

    element->release();
    element = 0;

    return element;
}

//---------------------------------------------------------------------------
// 

IOHIDElementPrivate *
IOHIDElementPrivate::collectionElement( IOHIDElementContainer *owner,
                                 IOHIDElementType      type,
                                 HIDCollectionExtendedNodePtr  collection,
                                 IOHIDElementPrivate *        parent )
{
	IOHIDElementPrivate * element = new IOHIDElementPrivate;

    // Check arguments and call init().

    if ( ( collection == 0 ) ||
         ( element    == 0 ) ||
         ( element->init( owner, type ) == false ) )
    {
        if ( element ) element->release();
        return 0;
    }

    // Set HID properties.

    element->_usagePage     = collection->collectionUsagePage;
    element->_usageMin      = collection->collectionUsage;
    element->_usageMax      = collection->collectionUsage;
    element->_collectionType = (IOHIDElementCollectionType)collection->data;
    
    element->_shouldTickleActivity = (element->_usagePage == kHIDPage_GenericDesktop);

    // Register with owner and parent.

    if ( ( owner->registerElement( element, &element->_cookie ) == false )
    ||   ( ( parent && ( parent->addChildElement(element) == false ) ) ) )
    {
        element->release();
        element = 0;
    }

    return element;
}

IOHIDElementPrivate *IOHIDElementPrivate::nullElement(IOHIDElementContainer *owner,
                                                      UInt32 reportID,
                                                      IOHIDElementPrivate *parent)
{
    IOHIDElementPrivate *element = new IOHIDElementPrivate;
    
    if (!element->init(owner, kIOHIDElementTypeInput_NULL)) {
        element->release();
        return NULL;
    }
    
    element->_reportID = reportID;
    owner->registerElement(element, &element->_cookie);
    
    if (parent) {
        parent->addChildElement(element);
    }
    
    return element;
}

//---------------------------------------------------------------------------
// 

IOHIDElementPrivate * IOHIDElementPrivate::reportHandlerElement(
                                            IOHIDElementContainer *owner,
                                            IOHIDElementType     type,
                                            UInt32               reportID,
                                            UInt32               reportBits )
{
    IOHIDElementPrivate * element = new IOHIDElementPrivate;

    if ( ( reportBits == 0 ) || ( element->init( owner, type ) == false ) )
    {
        element->release();
        return 0;
    }
    
    element->_isInterruptReportHandler	= true;
    element->_flags 			= kHIDDataVariable | kHIDDataRelative;
    element->_reportCount		= 1;
    element->_reportID			= reportID;
    element->_reportBits 		= element->_reportSize	= reportBits;
    element->_currentReportSizeBits = element->_reportBits * element->_reportCount;
    
    // Register with owner.

    if ( owner->registerElement( element, &element->_cookie ) == false )
    {
        element->release();
        element = 0;
    }
    
    return element;
}


//---------------------------------------------------------------------------
// 

IOHIDElementPrivate * IOHIDElementPrivate::newSubElement( UInt16 rangeIndex ) const
{
    IOHIDElementPrivate * element = new IOHIDElementPrivate;

    // Check arguments and call init().

    if ( (element == 0 ) ||
         ( element->init( _owner, _type ) == false ) )
    {
        if ( element ) element->release();
        return 0;
    }

    // Set HID properties.

    element->_flags          		= _flags;
    element->_reportID       		= _reportID;
    element->_usagePage      		= _usagePage;
    element->_usageMin       		= _usageMin;
    element->_usageMax       		= _usageMax;
    element->_rangeIndex     		= rangeIndex;
    element->_arrayReportHandler	= _arrayReportHandler;

    element->_reportBits     		= _reportBits;
    element->_reportStartBit 		= _reportStartBit + (rangeIndex * _reportBits);
    element->_logicalMin     		= _logicalMin;
    element->_logicalMax     		= _logicalMax;
    element->_physicalMin    		= _physicalMin;
    element->_physicalMax    		= _physicalMax;
    element->_units          		= _units;
    element->_unitExponent   		= _unitExponent;
    element->_rawReportCount        = _reportCount;
    element->_currentReportSizeBits = element->_reportBits * element->_reportCount;
	
    // RY: Special handling for array elements.
    // FYI, if this an array and button element, then we
    // know that this is a dummy array element.  The start
    // is not used to process the report, but instead used
    // to identify the which arrayHandler is belongs to.
    // Therefore, all subelements should contain the same
    // start bit.
    if ( IsArrayElement(this) && IsButtonElement(this) )
    {
        element->_reportStartBit = _reportStartBit;        
    }
    
    if (_duplicateElements)
    {        
        _duplicateElements->setObject(element);
        element->_duplicateReportHandler = _duplicateReportHandler;
    }

    // Register with owner and parent.

    if ( ( _owner->registerElement( element, &element->_cookie ) == false )
    ||   ( _parent && ( _parent->addChildElement(element) == false ) ) )
    {
        element->release();
        element = 0;
    }

    return element;
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::createSubElements()
{
    IOHIDElementPrivate * element;
    UInt32         count = getRangeCount();
    UInt32         index = getStartingRangeIndex();
    bool           ret = true;

    while ( index < count)
    {
        element = newSubElement( index++ );
        if ( element == 0 )
        {
            ret = false;
            break;
        }
        element->release();
    }

    return ret;
}

//---------------------------------------------------------------------------
//

void IOHIDElementPrivate::free()
{
    super::setOptions(0, kImmutable);
    
    if ( _childArray )
    {
        _childArray->release();
        _childArray = 0;
    }

    if ( _queueArray )
    {
        _queueArray->release();
        _queueArray = 0;
    }
    
    if ( _arrayItems )
    {
        _arrayItems->release();
        _arrayItems = 0;
    }
    
    if ( _duplicateElements )
    {
        _duplicateElements->release();
        _duplicateElements = 0;
    }
    
    if (_oldArraySelectors)
    {
        IODeleteData(_oldArraySelectors, UInt32, _reportCount);
    }

    if (_colArrayReportHandlers)
    {
        _colArrayReportHandlers->release();
        _colArrayReportHandlers = 0;
    }
    
    OSSafeReleaseNULL(_dataValue);
    
    super::free();
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::addChildElement( IOHIDElementPrivate * child, bool arrayHeader)
{

    if ( _childArray == 0 )
    {
        _childArray = OSArray::withCapacity(4);
    }

    if ( !_childArray ) 
        return false;
            
    // Perform special processing if this is an array item
    // that doesn't directly handle the report.  Basically,
    // we want to group all related array elements together.
    // This will help out for elements that are not part of 
    // a range.  Since collections can span multiple 
    // reports, we will use the following as a unique ID:
    //	    8bits: reportID
    //	   32bits: startBit
    //	   32bits: elementType
    if ( (child->_type != kIOHIDElementTypeCollection) &&
        IsArrayElement(child) &&
        !IsArrayElementTheReportHandler(child) && 
        (arrayHeader || !IsDuplicateElement(child)))
    {
        if (_colArrayReportHandlers ==0)
        {
            _colArrayReportHandlers = OSDictionary::withCapacity(1);
        }
        
        if (! _colArrayReportHandlers)
            return false;

        IOHIDElementPrivate *	arrayReportHandler;
        char		uniqueID[32];
        
        snprintf(uniqueID, sizeof(uniqueID), "%4.4x%4.4x%4.4x", (unsigned)child->_type, (unsigned)child->_reportStartBit, (unsigned)child->_reportID);
        
        arrayReportHandler = (IOHIDElementPrivate *)_colArrayReportHandlers->getObject(uniqueID);
        
        if (arrayReportHandler)
        {
            child->_arrayReportHandler = arrayReportHandler;
        }
        else
        {
            // We need to create array head based on info from
            // the child.
            arrayReportHandler = arrayHandlerElement(child->_owner, child->_type, child, this);
            
            if ( arrayReportHandler == 0 )
            {
                return false;
            }

            // Register this new element with this collection
            _colArrayReportHandlers->setObject(uniqueID, arrayReportHandler);
            arrayReportHandler->release();
        }
                
        // Now that we have the info from the child, revert
        // it back to a button.        
        child->_arrayReportHandler = arrayReportHandler;
        child->_reportBits 	   = 1;
        child->_reportCount        = 1;
        child->_logicalMin         = child->_physicalMin = 0;
        child->_logicalMax         = child->_physicalMax = 1;

        // Add the chile to the array list
        arrayReportHandler->_arrayItems->setObject(child);
    }

    _childArray->setObject( child );
    child->_parent = this;
    
    // RY: only override the child if you are not the root element
    if ( _cookie != 0 )
        child->_shouldTickleActivity = _shouldTickleActivity;

    return true;
}

IOHIDElementPrivate * IOHIDElementPrivate::arrayHandlerElement(                                
                                IOHIDElementContainer *owner,
                                IOHIDElementType type,
                                IOHIDElementPrivate * child,
                                IOHIDElementPrivate * parent)
{
    IOHIDElementPrivate * element = new IOHIDElementPrivate;

    // Check arguments and call init().

    if ( (element == 0 ) ||
        ( element->init( owner, type ) == false ) )
    {
        goto ARRAY_HANDLER_ELEMENT_RELEASE;
    }

    element->_arrayReportHandler = element;
    
    element->_parent         = parent;
    element->_flags          = child->_flags;
    element->_reportID       = child->_reportID;
    element->_usagePage      = child->_usagePage;
    element->_usageMin       = 0xffffffff;
    element->_usageMax       = 0xffffffff;        
    
    element->_reportBits     = child->_reportBits;
    element->_reportCount    = child->_reportCount;
    element->_reportStartBit = child->_reportStartBit;
    element->_logicalMin     = child->_logicalMin;
    element->_logicalMax     = child->_logicalMax;
    element->_physicalMin    = child->_physicalMin;
    element->_physicalMax    = child->_physicalMax;
    element->_rawReportCount = child->_reportCount;
    element->_currentReportSizeBits = child->_reportBits * child->_reportCount;
                
    // Allocate the array for the array elements.
    element->_arrayItems = OSArray::withCapacity((child->_usageMax - child->_usageMin) + 1);

    if (element->_arrayItems == NULL)
        goto ARRAY_HANDLER_ELEMENT_RELEASE;
        
    // RY: Allocate a buffer that will contain the old Array selector.
    // This needed to compare the old report to the new report to
    // deterine which array items need to be turned on/off.
    element->_oldArraySelectors = (UInt32 *)IONewZeroData(UInt32, element->_reportCount);
    
    if (element->_oldArraySelectors == NULL)
        goto ARRAY_HANDLER_ELEMENT_RELEASE;
    
    if (element->_reportCount > 1)
    {
        element->_duplicateReportHandler = element;
        element->_duplicateElements = OSArray::withCapacity(element->_reportCount);        
        
        if (element->_duplicateElements == NULL)
            goto ARRAY_HANDLER_ELEMENT_RELEASE;        
    }
                        
    if ( (owner->registerElement( element, &element->_cookie ) == true ) &&
        ( parent && ( parent->addChildElement(element) == true )) &&
        ( element->createSubElements() == true ))
    {
        return element;
    }
        
    
ARRAY_HANDLER_ELEMENT_RELEASE:
    element->release();
    element = 0;
    
    return element;
}

OSDictionary* IOHIDElementPrivate::createProperties() const
{
    UInt32          usage;
    
    OSDictionary *properties = OSDictionary::withCapacity(9);

    if (!properties) {
        goto done;
    }
    properties->setCapacityIncrement(15);

    usage = (_usageMax != _usageMin) ? _usageMin + _rangeIndex  : _usageMin;
    
#define SET_NUMBER(Y, Z) \
    do { \
        OSNumber *number = OSNumber::withNumber(Z, 32); \
        properties->setObject(Y, number); \
        number->release(); \
    } \
    while (false)
    
    SET_NUMBER(kIOHIDElementCookieKey, (UInt32) _cookie);
    SET_NUMBER(kIOHIDElementTypeKey, _type);
    SET_NUMBER(kIOHIDElementUsageKey, usage);
    SET_NUMBER(kIOHIDElementUsagePageKey, _usagePage);
    SET_NUMBER(kIOHIDElementReportIDKey, _reportID);
    SET_NUMBER(kIOHIDElementVariableSizeKey, _variableSize);
    
    if ( _type == kIOHIDElementTypeCollection ) {
        SET_NUMBER(kIOHIDElementCollectionTypeKey, _collectionType);
        goto done;
    }
    
    SET_NUMBER(kIOHIDElementSizeKey, (_reportBits * _reportCount));
    SET_NUMBER(kIOHIDElementReportSizeKey, _reportBits);
    SET_NUMBER(kIOHIDElementReportCountKey, _reportCount);
    
    if ( _isInterruptReportHandler ) {
        goto done;
    }
    
    SET_NUMBER(kIOHIDElementFlagsKey, _flags);
    SET_NUMBER(kIOHIDElementMaxKey, _logicalMax);
    SET_NUMBER(kIOHIDElementMinKey, _logicalMin);
    SET_NUMBER(kIOHIDElementScaledMaxKey, _physicalMax);
    SET_NUMBER(kIOHIDElementScaledMinKey, _physicalMin);
    SET_NUMBER(kIOHIDElementUnitKey, _units);
    SET_NUMBER(kIOHIDElementUnitExponentKey, _unitExponent);

#if 0
    
    SET_NUMBER(kIOHIDElementCalibrationMinKey, _calibration.min);
    SET_NUMBER(kIOHIDElementCalibrationMaxKey, _calibration.max);
    SET_NUMBER(kIOHIDElementCalibrationSaturationMinKey, _calibration.satMin);
    SET_NUMBER(kIOHIDElementCalibrationSaturationMaxKey, _calibration.satMax);
    SET_NUMBER(kIOHIDElementCalibrationDeadZoneMinKey, _calibration.dzMin);
    SET_NUMBER(kIOHIDElementCalibrationDeadZoneMaxKey, _calibration.dzMax);
    SET_NUMBER(kIOHIDElementCalibrationGranularityKey, _calibration.gran);
    
#endif
  
    if ( IsDuplicateElement(this) && !IsDuplicateReportHandler(this)) {
        SET_NUMBER(kIOHIDElementDuplicateIndexKey, _rangeIndex);
    }

    properties->setObject( kIOHIDElementHasNullStateKey, OSBoolean::withBoolean( _flags & kHIDDataNullState ));
    properties->setObject( kIOHIDElementHasPreferredStateKey, OSBoolean::withBoolean( !(_flags & kHIDDataNoPreferred) ));
    properties->setObject( kIOHIDElementIsNonLinearKey, OSBoolean::withBoolean( _flags & kHIDDataNonlinear ));
    properties->setObject( kIOHIDElementIsRelativeKey, OSBoolean::withBoolean( _flags & kHIDDataRelative ));
    properties->setObject( kIOHIDElementIsWrappingKey, OSBoolean::withBoolean( _flags & kHIDDataWrap ));
    properties->setObject( kIOHIDElementIsArrayKey, OSBoolean::withBoolean( IsArrayElement(this) ));

#undef SET_NUMBER
done:
    return properties;
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::serialize( OSSerialize * s ) const
{
    bool            ret = true;
    
    if ( !IsDuplicateElement(this) || IsDuplicateReportHandler(this) || (GetDuplicateElementCount(this) <= 32) ) {
        if (!s->previouslySerialized(this)) {
            OSDictionary *properties = createProperties();
            if ( properties ) {
                ret = properties->serialize(s);
                properties->release();
            }
            else {
                ret = false;
            }
        }
    }
    return ret;
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::fillElementStruct( IOHIDElementStruct * element )
{	 
    if ( (_usageMin != _usageMax) && (_rangeIndex >= 1) )
        return false;
        
    if ( IsDuplicateElement(this) )
    {
        if (!IsDuplicateReportHandler(this)) {
            return false;
        }
        
        IOHIDElementPrivate * dupElement;
        if (_duplicateElements && ( dupElement = (IOHIDElementPrivate *)_duplicateElements->getObject(0)))
        {
            element->duplicateValueSize = dupElement->getElementValueSize();
            element->duplicateIndex = 0xffffffff;
        }
    }

    element->cookieMin      = (UInt32)_cookie;
    element->cookieMax      = element->cookieMin + getRangeCount() - getStartingRangeIndex();
    element->parentCookie   = _parent ? (UInt32)_parent->_cookie : 0;
    element->type           = _type;
    element->collectionType = _collectionType;
    element->flags          = _flags;
    element->usagePage      = _usagePage;
    element->usageMin       = _usageMin;
    element->usageMax       = _usageMax;
    element->min            = _logicalMin;
    element->max            = _logicalMax;
    element->scaledMin      = _physicalMin;
    element->scaledMax      = _physicalMax;
    element->size           = _reportBits * _reportCount;
    element->reportSize     = _reportBits;
    element->reportCount    = _reportCount;
    element->rawReportCount = _rawReportCount;
    element->reportID       = _reportID;
    element->unit           = _units;
    element->unitExponent   = _unitExponent;
    element->bytes          = (UInt32)getByteSize();
    element->valueSize      = getElementValueSize();
    
    return true;
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::matchProperties(OSDictionary * matching)
{
    bool                ret        = true;
    OSDictionary      * properties = NULL;
    OSObject          * value      = NULL;
    static const char * keys[]     = {kIOHIDElementCookieKey, kIOHIDElementTypeKey, kIOHIDElementCollectionTypeKey, kIOHIDElementUsageKey, kIOHIDElementUsagePageKey, kIOHIDElementMinKey, kIOHIDElementMaxKey, kIOHIDElementScaledMaxKey, kIOHIDElementSizeKey, kIOHIDElementReportSizeKey, kIOHIDElementReportCountKey, kIOHIDElementIsArrayKey, kIOHIDElementIsRelativeKey, kIOHIDElementIsWrappingKey, kIOHIDElementIsNonLinearKey, kIOHIDElementHasPreferredStateKey, kIOHIDElementHasNullStateKey, kIOHIDElementVendorSpecificKey, kIOHIDElementUnitKey, kIOHIDElementUnitExponentKey, kIOHIDElementNameKey, kIOHIDElementValueLocationKey, kIOHIDElementDuplicateIndexKey, kIOHIDElementParentCollectionKey};

    require(matching, exit);
    require_action((properties = createProperties()), exit, ret = false);

    for (const char * key : keys) {
        value = matching->getObject(key);
        if (value && !value->isEqualTo(properties->getObject(key))) {
            ret = false;
            break;
        }
    }

exit:
    OSSafeReleaseNULL(properties);
    return ret;
}

//---------------------------------------------------------------------------
// 
UInt32 IOHIDElementPrivate::getElementValueSize() const
{
    UInt32  size        = sizeof(IOHIDElementValue);
    UInt32  reportWords = (_reportBits * _reportCount) / (sizeof(UInt32) * 8);
    
    // RY: Don't forget the remainder.
    reportWords += ((_reportBits * _reportCount) % (sizeof(UInt32) * 8)) ? 1 : 0;

    if ( reportWords > 1 )
    {
        size += ((reportWords - 1) * sizeof(UInt32));
    }

    return size;
}


//---------------------------------------------------------------------------
// Not very efficient, will do for now.

#define BIT_MASK(bits)  ((1UL << (bits)) - 1)

#define UpdateByteOffsetAndShift(bits, offset, shift)  \
    do { offset = bits >> 3; shift = bits & 0x07; } while (0)

#define UpdateWordOffsetAndShift(bits, offset, shift)  \
    do { offset = bits >> 5; shift = bits & 0x1f; } while (0)

static void readReportBits( const UInt8 * src,
                           UInt32 *      dst,
                           UInt32        bitsToCopy,
                           UInt32        srcStartBit = 0,
                           bool          shouldSignExtend = false,
                           bool *        valueChanged = 0)
{
    UInt32 srcOffset;
    UInt32 srcShift;
    UInt32 dstShift      = 0;
    UInt32 dstStartBit   = 0;
    UInt32 dstOffset     = 0;
    UInt32 lastDstOffset = 0;
    UInt32 word          = 0;
    UInt8  bitsProcessed;
	UInt32 totalBitsProcessed = 0;

    UpdateByteOffsetAndShift( srcStartBit, srcOffset, srcShift );

    if (srcStartBit % 8 == 0 && bitsToCopy % 8 == 0 && !shouldSignExtend) {
        bool changed = memcmp((dst + dstOffset), (src + srcOffset), bitsToCopy / 8) != 0;
        if (changed) {
            memcpy((dst + dstOffset), (src + srcOffset), bitsToCopy / 8);
        }
        if (valueChanged) {
            *valueChanged = changed;
        }
        return;
    }

    while ( bitsToCopy )
    {
        UInt32 tmp;

        UpdateByteOffsetAndShift( srcStartBit, srcOffset, srcShift );

        bitsProcessed = min( bitsToCopy,
                             min( 8 - srcShift, 32 - dstShift ) );

        tmp = (src[srcOffset] >> srcShift) & BIT_MASK(bitsProcessed);

        word |= ( tmp << dstShift );

        dstStartBit += bitsProcessed;
        srcStartBit += bitsProcessed;
        bitsToCopy  -= bitsProcessed;
		totalBitsProcessed += bitsProcessed;

        UpdateWordOffsetAndShift( dstStartBit, dstOffset, dstShift );

        if ( ( dstOffset != lastDstOffset ) || ( bitsToCopy == 0 ) )
        {
            // sign extend negative values
			// if this is the leftmost word of the result
			if ((lastDstOffset == 0) &&
				// and the logical min or max is less than zero
				// so we should sign extend
				(shouldSignExtend))
			{
				// SInt32 temp = word;
				
				// is this less than a full word
				if ((totalBitsProcessed < 32) && 
					// and the value negative (high bit set)
					(word & (1 << (totalBitsProcessed - 1))))
					// or in all 1s above the significant bit
					word |= ~(BIT_MASK(totalBitsProcessed));
			}

			
			if ( dst[lastDstOffset] != word )
            {
                dst[lastDstOffset] = word;
				if (valueChanged) {
                    *valueChanged = true;
                }
            }
            word = 0;
            lastDstOffset = dstOffset;
        }
    }
}

static void writeReportBits( const UInt32 * src,
                           UInt8 *        dst,
                           UInt32         bitsToCopy,
                           UInt32         dstStartBit = 0)
{
    UInt32 dstOffset;
    UInt32 dstShift;
    UInt32 srcShift    = 0;
    UInt32 srcStartBit = 0;
    UInt32 srcOffset   = 0;
    UInt8  bitsProcessed;
    UInt32 tmp;

    UpdateByteOffsetAndShift( dstStartBit, dstOffset, dstShift );

    if (dstStartBit % 8 == 0 && bitsToCopy % 8 == 0) {
        memcpy((dst + dstOffset), (src + srcOffset), bitsToCopy / 8);
        return;
    }

    while ( bitsToCopy )
    {
        UpdateByteOffsetAndShift( dstStartBit, dstOffset, dstShift );

        if (dstStartBit % 8 == 0 && bitsToCopy % 8 == 0) {
            memcpy((dst + dstOffset), (src + srcOffset), bitsToCopy / 8);
            break;
        }

        bitsProcessed = min( bitsToCopy,
                             min( 8 - dstShift, 32 - srcShift ) );

        tmp = (src[srcOffset] >> srcShift) & BIT_MASK(bitsProcessed);

        dst[dstOffset] |= ( tmp << dstShift );

        dstStartBit += bitsProcessed;
        srcStartBit += bitsProcessed;
        bitsToCopy  -= bitsProcessed;

        UpdateWordOffsetAndShift( srcStartBit, srcOffset, srcShift );
    }
}

bool IOHIDElementPrivate::enqueueValue(IOHIDElementValue *value)
{
    bool result = false;
    IOHIDEventQueue *queue = NULL;
    IOHIDReportElementQueue *reportQueue = NULL;

    require(_queueArray, exit);

    for (UInt32 i = 0; i < _queueArray->getCount(); i++) {
        queue = OSDynamicCast(IOHIDEventQueue, _queueArray->getObject(i));
        if (!queue) {
            continue;
        }

        reportQueue = OSDynamicCast(IOHIDReportElementQueue, queue);
        if (reportQueue) {
            result = reportQueue->enqueue(value);
        } else {
            result = queue->enqueue(value, value->totalSize);
        }

        if (!result) {
            IOHID_DEBUG(kIOHIDDebugCode_HIDDeviceEnqueueFail, mach_continuous_time(), 0, 0, 0);
        }
    }
    
exit:
    return result;
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::processReport(
                                    UInt8                       reportID,
                                    void *                      reportData,
                                    UInt32                      reportBits,
                                    const AbsoluteTime *        timestamp,
                                    IOHIDElementPrivate **      next,
                                    IOOptionBits                options)
{
    IOHIDEventQueue *   queue;
    bool				changed = false;
    
    if (_type == kIOHIDElementTypeInput_NULL
        && reportID == _reportID) {
        _elementValue->timestamp = *timestamp;
        enqueueValue(_elementValue);
        *next = NULL;
        goto exit;
    }

    // Set next pointer to the next report handler in the chain.
    // If this is an array, set the report handler to the one
    // the array.
    if (next)
    {
        *next = _nextReportHandler;

        if ( _reportID != reportID )
        {
            return false;
        }

        // Verify incoming report size.
 
        if (!_variableSize && _reportSize && (reportBits < _reportSize))
        {
            *next = 0;
            return false;
        }
        
        if (_isInterruptReportHandler && (options & kIOHIDReportOptionNotInterrupt))
        {
            return false;
        }
        
        if (IsArrayElement(this) && !IsArrayReportHandler(this))
        {
            *next = _arrayReportHandler;
            return false;
        }
        
    }
    
    do {
        // Ignore report that does not match our report ID.
        if ( _reportID != reportID )
            break;

        if (_variableSize & kIOHIDElementVariableSizeElement) {
            if (_reportStartBit >= reportBits) {
                break;
            }
        } else if ((_reportStartBit + (_reportBits * _reportCount)) > reportBits) {
            break;
        }
      
        if ( ( _usagePage == kHIDPage_KeyboardOrKeypad )
             && ( getUsage() >= kHIDUsage_KeyboardLeftControl )
             && ( getUsage() <= kHIDUsage_KeyboardRightGUI )
             && _rollOverElementPtr
             && *_rollOverElementPtr
             && (*_rollOverElementPtr)->getValue())
        {
            AbsoluteTime rollOverTS = (*_rollOverElementPtr)->getTimeStamp();
            if ( CMP_ABSOLUTETIME(&rollOverTS, timestamp) == 0 )
                break;
        }
        
        // The generation is incremented before and after
        // processing the report.  An odd value tells us
        // that the information is incomplete and should
        // not be trusted.  An even value tells us that
        // the value is complete.
        _elementValue->generation++;

        _previousValue = _elementValue->value[0];
		
        // Get the element value from the report.
        uint32_t readSize;
        if (os_mul_overflow(_reportBits, _reportCount, &readSize)) {
            HIDLogError("Overflow calculating readsize");
            break;
        }
        if (_variableSize & kIOHIDElementVariableSizeElement) {
          uint32_t remainingBitSize = reportBits - _reportStartBit;
          readSize = (remainingBitSize < readSize) ? remainingBitSize : readSize;
        }

        readReportBits( (UInt8 *) reportData,  /* source buffer      */
                       _elementValue->value,   /* destination buffer */
                       readSize,               /* bits to copy       */
                       _reportStartBit,        /* source start bit   */
                       (((SInt32)_logicalMin < 0) || ((SInt32)_logicalMax < 0)), /* should sign extend */
                       &changed );             /* did value change?  */
      
        _currentReportSizeBits = readSize;
        // Set a timestamp to indicate the last modification time.
        // We should set the time stamp if the generation is 1 regardless if the value
        // changed.  This will insure that an initial value of 0 will have the correct
        // timestamp
        do {
            bool shouldProcess = (changed || _isInterruptReportHandler || (_flags & kHIDDataRelativeBit));
            
            if ( shouldProcess ) {
                // Let's not update the timestamp in the case where the element is relative, and there is no change
                if (((_flags & kHIDDataRelativeBit) == 0) || (_reportBits > 32) || changed || _previousValue)
                    _elementValue->timestamp = *timestamp;
                    
                if (IsArrayElement(this) && IsArrayReportHandler(this))
                    processArrayReport(reportID, reportData, reportBits, &(_elementValue->timestamp));
            }
            
            // Update element for first time
            // This may be invalid report since timestamp is still 0
            // so we should just update timestamp and not dispatch any report
            if (_elementValue && _elementValue->timestamp == 0 && timestamp) {
                _elementValue->timestamp = *timestamp;
            }
            
            if ( !_queueArray )
                break;
                
            for ( UInt32 i = 0; _queueArray && (queue = (IOHIDEventQueue *) _queueArray->getObject(i)); i++ )
            {
                // Enqueue may block for some clients, retain the queue here to prevent it from disappearing before we finish the enqueue.
                queue->retain();
                //Pass actual element size. (different fotr variable lenght reports)
                _elementValue->totalSize = (_currentReportSizeBits + 7) / 8 + ELEMENT_VALUE_HEADER_SIZE(_elementValue);
                //enqueueSize dword aligned
                uint32_t  enqueueSize = ALIGN_DATA_SIZE(_elementValue->totalSize);
                if ( shouldProcess || (queue->getOptions() & kIOHIDQueueOptionsTypeEnqueueAll)) {
                    bool result;
                    IOHIDReportElementQueue *elementQueue = OSDynamicCast(IOHIDReportElementQueue, queue);
                    if (elementQueue) {
                        result = elementQueue->enqueue(_elementValue);
                    } else {
                        result = queue->enqueue( (void*) _elementValue, enqueueSize);
                    }
                    if (!result) {
                        IOHID_DEBUG(kIOHIDDebugCode_HIDDeviceEnqueueFail, mach_continuous_time(), 0, 0, 0);
                    }
                }
                queue->release();
            }
        } while ( 0 );

        _elementValue->generation++;
        
        // If this element is part of a transaction
        // set its state to idle
        if (_transactionState)
            _transactionState = kIOHIDTransactionStateIdle;
    }
    while ( false );

exit:
    return changed;
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::createReport( UInt8           reportID,
                                 void *		 reportData,  // this report should be alloced outisde this method.
                                 UInt32 *        reportLength,
                                 IOHIDElementPrivate ** next )
{
    bool handled = false;
    
    if (_type == kIOHIDElementTypeInput_NULL
        && reportID == _reportID) {
        *next = NULL;
        goto exit;
    }

    if (next)
        *next = _nextReportHandler;

    do {
        // Ignore report that does not match our report ID.
    
        if ( _reportID != reportID )
            break;

        //------------------------------------------------
        // Changed this portion of the method.
        // The report is now allocated outside of the 
        // method
         
        if ( _reportSize )
        {
            *reportLength = _reportSize / 8;            

            if ( reportData == 0 )
            {
                if (next) *next = 0;
                break;
            }

            bzero( reportData, *reportLength );
        } 
        
        //------------------------------------------------
        

        // Set next pointer to the next report handler in the chain.
        // If this is an array or duplicate, set the next to the 
        // appropriate handler.
        if (next)
        {            
            if (IsArrayElement(this))
            {
                if (!IsArrayReportHandler(this))
                {
                    *next = _arrayReportHandler;
                    break;
                }
                
                // RY: Only bother creating an array report is this element
                // is idle.
                if (_transactionState == kIOHIDTransactionStateIdle)
                    return createArrayReport(reportID, reportData, reportLength);
            }            
            else if (IsDuplicateElement(this))
            {
                if (!IsDuplicateReportHandler(this))
                {
                    *next = _duplicateReportHandler;
                    break;
                }
                
                // RY: Only bother creating a report if the duplicate report
                // elements are idle.                
                if (_transactionState == kIOHIDTransactionStateIdle)
                    return createDuplicateReport(reportID, reportData, reportLength);
            }
        }

        // If this element has not been set, an out of bounds
        // value must be set.  This will cause the device
        // to ignore the report for this element.
        if ( _transactionState == kIOHIDTransactionStateIdle )
        {
                setOutOfBoundsValue();
        }

        // Set the element value to the report.
        if ( reportData )
        {
            writeReportBits( _elementValue->value,   	/* source buffer      */
                           (UInt8 *) reportData,  	/* destination buffer */
                           (_reportBits * _reportCount),/* bits to copy       */
                           _reportStartBit);       	/* dst start bit      */                           

            handled = true;
            
            // Clear the transaction state
            _transactionState = kIOHIDTransactionStateIdle;
        }
        
    }
    while ( false );

exit:
    return handled;
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::setMemoryForElementValue(
                                    IOVirtualAddress        address,
                                    void *                  location)
{
    _elementValue = (IOHIDElementValue *) address;
    _elementValueLocation = location;

    // Clear memory block, and set the invariants.

    bzero( _elementValue, getElementValueSize() );

	_elementValue->cookie    = _cookie;
	_elementValue->totalSize = getElementValueSize();

    return true;
}

//---------------------------------------------------------------------------
// Return the number of elements in a usage range.

UInt32 IOHIDElementPrivate::getRangeCount() const
{
    // FIXME - shouldn't we use logical min/max?

    // Check to see if we have multiple elements with the same usage.
    // If so, return the _reportCount
    return (_reportCount > 1) ? _reportCount : (_usageMax-_usageMin + 1 );
}

//---------------------------------------------------------------------------
// Return the number of elements in a usage range.

UInt32 IOHIDElementPrivate::getStartingRangeIndex() const
{
    // Check to see if we have multiple elements with the same usage.
    return (_reportCount > 1) ? 0 : 1;
}


//---------------------------------------------------------------------------
// 

IOHIDElementPrivate *
IOHIDElementPrivate::setNextReportHandler( IOHIDElementPrivate * element )
{
    IOHIDElementPrivate * prev = _nextReportHandler;
    _nextReportHandler  = element;
    return prev;
}


//---------------------------------------------------------------------------
// 

void IOHIDElementPrivate::setRollOverElementPtr( IOHIDElementPrivate ** elementPtr )
{
    _rollOverElementPtr = elementPtr;
}


//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::getReportType( IOHIDReportType * reportType ) const
{
    if ( _type <= kIOHIDElementTypeInput_NULL )
        *reportType = kIOHIDReportTypeInput;
    else if ( _type == kIOHIDElementTypeOutput )
        *reportType = kIOHIDReportTypeOutput;
    else if ( _type == kIOHIDElementTypeFeature )
        *reportType = kIOHIDReportTypeFeature;
    else
        return false;

    return true;
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::addEventQueue( IOHIDEventQueue * queue )
{
    if ( !queue )
        return false;
        
    if ( _queueArray == 0 )
    {
        _queueArray = OSArray::withCapacity(4);
    }

    if ( hasEventQueue(queue) == true )
        return false;

    return _queueArray ? _queueArray->setObject( queue ) : false;
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::removeEventQueue( IOHIDEventQueue * queue )
{
    OSObject * obj = 0;
    
    if ( !queue )
        return false;

    for ( UInt32 i = 0;
          _queueArray && (obj = _queueArray->getObject(i)); i++ )
    {
        if ( obj == (OSObject *) queue )
        {
            _queueArray->removeObject(i);
            if ( _queueArray->getCount() == 0 )
            {
                _queueArray->release();
                _queueArray = 0;
            }
            break;
        }
    }

    return (obj != 0);
}

//---------------------------------------------------------------------------
// 

bool IOHIDElementPrivate::hasEventQueue( IOHIDEventQueue * queue )
{
    OSObject * obj = 0;

    for ( UInt32 i = 0;
          _queueArray && (obj = _queueArray->getObject(i)); i++ )
    {
        if ( obj == (OSObject *) queue )
            break;
    }

    return (obj != 0);
}

//---------------------------------------------------------------------------
// 

UInt32 IOHIDElementPrivate::setReportSize( UInt32 numberOfBits )
{
    UInt32 oldSize = _reportSize;
    _reportSize = numberOfBits;
    return oldSize;
}

//---------------------------------------------------------------------------
// This methods will set an out of bounds element value.  This value will
// be based on the _logicalMin or _logicalMax depending on bit space.  If
// no room is available to go outside the range, the value will remain 
// unchanged.
void IOHIDElementPrivate::setOutOfBoundsValue()
{
    // Make sure we are not dealing with long element value type
    if ( _elementValue->totalSize == sizeof(IOHIDElementValue)) {
    
        // Simple case:  If the _logicalMin > 0, then we can just
        // set the elementValue to 0
        if ( _logicalMin > 0 ) {
            _elementValue->value[0] = 0;
        }
        else {
            // Make sure there is room
            if ( ( (_logicalMax - _logicalMin) + 1) < (1 << _reportBits))
            {
                // handle overflow
                if ( ( (_logicalMax + 1) & BIT_MASK(_reportBits) ) == (_logicalMax + 1) )
                    _elementValue->value[0] = _logicalMax + 1;
                else 
                    _elementValue->value[0] = _logicalMin - 1;
            }
        }
    }
}

bool IOHIDElementPrivate::createDuplicateReport(UInt8		reportID,
                                        void *		reportData,
                                        UInt32 *	reportLength)
{
    bool		pending = false;
    IOHIDElementPrivate 	*element;

    // RY: Then, check the other duplicates to see if they are currently 
    // pending.  
    for (unsigned i=0;  _duplicateElements && i<_duplicateElements->getCount(); i++) 
    {
        element = (IOHIDElementPrivate *)_duplicateElements->getObject(i);
        if (element->_transactionState == kIOHIDTransactionStatePending)
        {
            pending = true;
        }
        
        element->createReport(reportID, reportData, reportLength, 0);
    }
    
    return pending;
}

bool IOHIDElementPrivate::createArrayReport(UInt8	reportID,
                                    void *	reportData,
                                    UInt32 *	reportLength) 
{
    IOHIDElementPrivate 	*element, *arrayElement;
    UInt32		arraySel;
    UInt32		i, reportIndex = 0;
    
    if (createDuplicateReport(reportID, reportData, reportLength))
        return true;
                        
    for (i=0; i<_arrayItems->getCount(); i++)
    {
        element = (IOHIDElementPrivate *)(_arrayItems->getObject(i));

        if (!element)
            continue;
        if ( element->_transactionState == kIOHIDTransactionStateIdle )
            continue;
            
        if (element->_elementValue->value[0] == 0)
            continue;

        arraySel = GetArrayItemSel(i);

        if ( NULL != (arrayElement = ((_duplicateElements) ? (IOHIDElementPrivate *)_duplicateElements->getObject(reportIndex) : this)) )
        {
            arrayElement->_elementValue->value[0] = arraySel;
            arrayElement->_transactionState = kIOHIDTransactionStatePending;
            arrayElement->createReport(reportID, reportData, reportLength, 0);
        }
        
        reportIndex ++;
        
        element->_transactionState = kIOHIDTransactionStateIdle;
        
        // Make sure we don't add to many usages to the report
        if (reportIndex >= _reportCount)
            break;
    }
    
    // Clear out the remaining portions of the report for this array
    arraySel = 0;
    for (i=reportIndex; i<_reportCount; i++)
    {        
        if ( NULL != (arrayElement = ((_duplicateElements) ? (IOHIDElementPrivate *)_duplicateElements->getObject(reportIndex) : this)) )
        {
            arrayElement->_elementValue->value[0] = arraySel;
            arrayElement->_transactionState = kIOHIDTransactionStatePending;
            arrayElement->createReport(reportID, reportData, reportLength, 0);
        }
    }
    
    return true;
}

void IOHIDElementPrivate::setArrayElementValue(UInt32 index, UInt32 value)
{
    IOHIDElementPrivate 	*element;
    
    if ( !_arrayItems || (index > _arrayItems->getCount()))
        return;
        
    element = (IOHIDElementPrivate *)(_arrayItems->getObject(index));
    
    if (!element) 
        return;

    // Avoid setting the same element value twice in the same report.
    // Prevents accidentally overwriting the previous value if 2 array
    // indicies were reporting the same element, and are now both zero.
    if (CMP_ABSOLUTETIME(&element->_elementValue->timestamp, &_elementValue->timestamp) == 0) {
        return;
    }

    // Bump the generation count.  An odd value tells us
    // that the information is incomplete and should not
    // be trusted.  An even value tells us that the value
    // is complete. 
    element->_elementValue->generation ++;
    
    element->_previousValue = element->_elementValue->value[0];
    element->_elementValue->value[0] = value;
    element->_elementValue->timestamp = _elementValue->timestamp;
    
    element->_elementValue->generation ++;

    element->enqueueValue(element->_elementValue);
}

bool IOHIDElementPrivate::processArrayReport(	UInt8			reportID,
                                        void *			reportData,
                                        UInt32			reportBits,
                                        const AbsoluteTime *	timestamp)
{
    IOHIDElementPrivate *	element		= NULL;
    UInt32		arraySel	= 0;
    UInt32		iNewArray	= 0;
    UInt32		iOldArray	= 0;
    bool		found		= false;
    bool		changed		= false;

    // RY: Process the arry selector elements.  If any of their values
    // haven't changed, don't bother with any further processing.  
    if (_duplicateElements)
    {
        bool keyboard = found = (_usagePage == kHIDPage_KeyboardOrKeypad);
        for (iNewArray = 0; iNewArray < _reportCount; iNewArray ++)
        {
            if ( NULL != (element = (IOHIDElementPrivate *)_duplicateElements->getObject(iNewArray)) )
            {
                changed |= element->processReport(reportID, reportData, reportBits, timestamp, 0);
                if (keyboard && (element->_elementValue->value[0] != kHIDUsage_KeyboardErrorRollOver))
                {
                    found = false;
                }
            }
        }
        
        if (!changed)
            return changed;
        else if (keyboard)
        {
            setArrayElementValue(GetArrayItemIndex(kHIDUsage_KeyboardErrorRollOver), (found ? 1 : 0));
            
            if (found)
                return false;
        }
    }
                                    
    // Check the existing indexes against the originals
    for (iOldArray = 0; iOldArray < _reportCount; iOldArray ++)
    {
        arraySel = _oldArraySelectors[iOldArray];
        
        found = false;

        for (iNewArray = 0; iNewArray < _reportCount; iNewArray ++)
        {
            element = (_duplicateElements) ? (IOHIDElementPrivate *)_duplicateElements->getObject(iNewArray) : this;
            if (element && (arraySel == element->_elementValue->value[0]))
            {
                found = true;
                break;
            }
        }
        
        // The index is no longer present.  Set its value to 0.
        if (!found)
            setArrayElementValue(GetArrayItemIndex(arraySel), 0);
    }
    
    // Now add new indexes to _oldArraySelectors
    for (iNewArray = 0; iNewArray < _reportCount; iNewArray ++)
    {
        if (!(element = (_duplicateElements) ? (IOHIDElementPrivate *)_duplicateElements->getObject(iNewArray) : this))
            continue;
            
        arraySel = element->_elementValue->value[0];
                
        found = false;

        for (iOldArray = 0; iOldArray < _reportCount; iOldArray ++)
        {
            if (arraySel == _oldArraySelectors[iOldArray])
            {
                found = true;
                break;
            }
        }
        
        // This is a new index.  Set its value to 1.
        if (!found)
            setArrayElementValue(GetArrayItemIndex(arraySel), 1);
    }
            
    // save the new array to _oldArraySelectors for future reference
    for (iOldArray = 0; iOldArray < _reportCount; iOldArray ++)
    {
        if ( NULL != (element = ((_duplicateElements) ? (IOHIDElementPrivate *)_duplicateElements->getObject(iOldArray) : this)) )
        _oldArraySelectors[iOldArray] = element->_elementValue->value[0];
    }

    return changed;
}

IOHIDElementCookie IOHIDElementPrivate::getCookie()
{   return _cookie;   }

IOHIDElementType IOHIDElementPrivate::getType()
{   return _type;   }

IOHIDElementCollectionType IOHIDElementPrivate::getCollectionType()
{   return _collectionType;   }

OSArray * IOHIDElementPrivate::getChildElements()
{   return _childArray;   }

IOHIDElement * IOHIDElementPrivate::getParentElement()
{   return _parent;   }

UInt32 IOHIDElementPrivate::getUsagePage()
{   return _usagePage;   }

UInt32 IOHIDElementPrivate::getUsage()
{
	return (_usageMax != _usageMin) ? _usageMin + _rangeIndex  : _usageMin;
}

UInt32 IOHIDElementPrivate::getReportID()
{   return _reportID;   }

UInt32 IOHIDElementPrivate::getReportSize()
{   return _reportBits;   }

UInt32 IOHIDElementPrivate::getReportCount()
{   return _reportCount;   }

UInt32 IOHIDElementPrivate::getFlags()
{   return _flags;  }

UInt32 IOHIDElementPrivate::getLogicalMin()
{   return _logicalMin;   }

UInt32 IOHIDElementPrivate::getLogicalMax()
{   return _logicalMax;   }

UInt32 IOHIDElementPrivate::getPhysicalMin()
{   return _physicalMin;   }

UInt32 IOHIDElementPrivate::getPhysicalMax()
{   return _physicalMax;   }

UInt32 IOHIDElementPrivate::getUnit()
{   return _units;   }

UInt32 IOHIDElementPrivate::getUnitExponent()
{   return _unitExponent;   }

UInt32 IOHIDElementPrivate::getValue()
{   
    return getValue(0);
}

OSData * IOHIDElementPrivate::getDataValue()
{   
    UInt32 byteSize = (UInt32)getCurrentByteSize();
    
#if defined(__LITTLE_ENDIAN__)
    if ( _dataValue && _dataValue->getLength() == byteSize) {
        bcopy((const void *)_elementValue->value, (void *)_dataValue->getBytesNoCopy(), byteSize);
    } else {
        OSSafeReleaseNULL(_dataValue);
        _dataValue = OSData::withBytes((const void *)_elementValue->value, byteSize);
    }
#else
    UInt32 bitsToCopy =  _currentReportSizeBits;
    if ( !_dataValue || _dataValue->getLength() != byteSize) {
        UInt8 * bytes[byteSize];
        OSSafeReleaseNULL(_dataValue);
        _dataValue = OSData::withBytes(bytes, byteSize);
    }

    if ( _dataValue ) {
        bzero((void *)_dataValue->getBytesNoCopy(), byteSize);
        writeReportBits((const UInt32*)_elementValue->value, (UInt8 *)_dataValue->getBytesNoCopy(), bitsToCopy);
    }
#endif
    return _dataValue;
}

OSData * IOHIDElementPrivate::getDataValue(IOOptionBits options)
{
    if (options & kIOHIDValueOptionsUpdateElementValues) {
        IOReturn status = _owner->updateElementValues(&_cookie, 1);
        if (status) {
            HIDLogError("getDataValue failed (%lu):%x", (uintptr_t)_cookie, status);
        }
    }
    return getDataValue();
}

void IOHIDElementPrivate::setValue(UInt32 value)
{ 
    setValue(value, 0);
}

void IOHIDElementPrivate::setValue(UInt32 value, IOOptionBits options)
{
    UInt32  previousValue = _elementValue->value[0];

    // If element has not been updated it's generation count
    // would be 0, we shouldn't block first attempt to
    // write element value

    if ((previousValue == value && !(options & kIOHIDValueOptionsUpdateElementValues)) && _elementValue->generation > 0) {
        return;
    }

    _elementValue->generation++;

    _elementValue->value[0] = value;

    IOReturn status = _owner->postElementValues(&_cookie, 1);
    if (status) {
        HIDLogError("setValue failed (%lu):%x", (uintptr_t)_cookie, status);
        _elementValue->value[0] = previousValue;
    } else {
        _previousValue = previousValue;
    }

    _elementValue->generation++;
}

void IOHIDElementPrivate::setDataValue(OSData * value)
{
    OSData * previousValue;
    
    if ( !value ) return;
    
    previousValue = getDataValue();
    
    setDataBits(value);
    
    IOReturn status = _owner->postElementValues(&_cookie, 1);
    if (status) {
        HIDLogError("setDataValue failed (%lu):%x", (uintptr_t)_cookie, status);
        setDataBits(previousValue);
    }
}

void IOHIDElementPrivate::setDataBits(OSData *value)
{
    UInt32  bitsToCopy;

    if ( !value || !value->getBytesNoCopy() ) return;

    bitsToCopy = min ( (value->getLength() << 3), (_reportBits * _reportCount) );
	
    readReportBits((const UInt8*)value->getBytesNoCopy(), _elementValue->value, bitsToCopy);
}

AbsoluteTime IOHIDElementPrivate::getTimeStamp()
{
	return _elementValue->timestamp;
}

IOByteCount IOHIDElementPrivate::getByteSize() const
{
    IOByteCount byteSize;
    UInt32      bitCount = _reportBits * _reportCount;
    
    byteSize = bitCount >> 3;
    byteSize += (bitCount % 8) ? 1 : 0;
	
    return byteSize;
}

IOByteCount IOHIDElementPrivate::getCurrentByteSize()
{
    IOByteCount byteSize;
    UInt32      bitCount = _currentReportSizeBits;

    byteSize = bitCount >> 3;
    byteSize += (bitCount % 8) ? 1 : 0;

    return byteSize;
}

unsigned int IOHIDElementPrivate::iteratorSize() const
{
    return 0;
}

bool IOHIDElementPrivate::initIterator(void * iterationContext __unused) const
{
    return false;
}

bool IOHIDElementPrivate::getNextObjectForIterator(void      * iterationContext __unused,
                                                   OSObject ** nextObject) const
{
    *nextObject = NULL;
    return 0;
}

unsigned int IOHIDElementPrivate::getCount() const
{
    return 1;
}

unsigned int IOHIDElementPrivate::getCapacity() const
{
    return 1;
}

unsigned int IOHIDElementPrivate::getCapacityIncrement() const
{
    return 0;
}

unsigned int IOHIDElementPrivate::setCapacityIncrement(unsigned increment __unused)
{
    return 0;
}

unsigned int IOHIDElementPrivate::ensureCapacity(unsigned int newCapacity __unused)
{
    return 0;
}

void IOHIDElementPrivate::flushCollection()
{
}

unsigned IOHIDElementPrivate::setOptions(unsigned options,
                                         unsigned mask,
                                         void * context __unused)
{
    unsigned old = super::setOptions(options, mask);
    if ((old ^ options) & mask) {
        // Value changed need to set all of the child collections
        if (_childArray)
            _childArray->setOptions(options, mask);
    }
    return old;
}

OSCollection * IOHIDElementPrivate::copyCollection(OSDictionary * cycleDict)
{
    bool            allocDict = !cycleDict;
    OSCollection    *result = NULL;
    OSDictionary    *properties = NULL;
    
    if (allocDict) {
        cycleDict = OSDictionary::withCapacity(16);
        require(cycleDict, done);
    }
    
    // Check for a cycle
    result = super::copyCollection(cycleDict);
    if (result)
        goto done;
    
    properties = createProperties();
    require(properties, done);
    if (_childArray) {
        if (_childArray->getCount() < 0x1000) {
            OSCollection *childCopy = _childArray->copyCollection(cycleDict);
            if (childCopy) {
                properties->setObject( kIOHIDElementKey, childCopy );
                childCopy->release();
            }
        }
        else {
            char buffer[256] = "";
            OSObject *str = NULL;
            snprintf(buffer, sizeof(buffer), "Attempted to get %s on an element with %d children",
                     kIOHIDElementKey, _childArray->getCount());
            HIDLogError("%s", buffer);
            str = OSString::withCString(buffer);
            if (str) {
                properties->setObject( kIOHIDElementKey, str );
                str->release();
            }
            else {
                properties->setObject( kIOHIDElementKey, kOSBooleanFalse );
            }
        }
    }
    
    // Insert object into cycle Dictionary
    cycleDict->setObject((const OSSymbol *) this, properties);
    
    result = properties;
    properties = 0;
    
done:
    if (allocDict && cycleDict)
        cycleDict->release();
    return result;
}

bool IOHIDElementPrivate::conformsTo(UInt32 usagePage, UInt32 usage)
{
    IOHIDElement * element = this;
    bool conforms = false;
    
    do {
        if ( usagePage != element->getUsagePage() )
            continue;
        
        if ( usage && usage != element->getUsage() )
            continue;
        
        conforms = true;
        break;
        
    } while ( (element = element->getParentElement()) );
    
    return conforms;
}

void IOHIDElementPrivate::setCalibration(UInt32 min, UInt32 max, UInt32 saturationMin, UInt32 saturationMax, UInt32 deadZoneMin, UInt32 deadZoneMax, IOFixed granularity)
{
    _calibration.satMin = saturationMin;
    _calibration.satMax = saturationMax;
    _calibration.dzMin  = deadZoneMin;
    _calibration.dzMax  = deadZoneMax;
    _calibration.min    = min;
    _calibration.max    = max;
    _calibration.gran   = granularity;
}

UInt32 IOHIDElementPrivate::getScaledValue(IOHIDValueScaleType type)
{
    SInt64  logicalValue    = (SInt32)getValue();
    SInt64  logicalMin      = (SInt32)getLogicalMin();
    SInt64  logicalMax      = (SInt32)getLogicalMax();
    SInt64  logicalRange    = 0;
    SInt64  scaledMin       = 0;
    SInt64  scaledMax       = 0;
    SInt64  scaledRange     = 0;
    SInt64  returnValue     = 0;

    if ( type == kIOHIDValueScaleTypeCalibrated ){
        
        if ( _calibration.min != _calibration.max ) {
            scaledMin = _calibration.min;
            scaledMax = _calibration.max;
        } else {
            scaledMin = -1;
            scaledMax = 1;
        }
        
        // check saturation first
        if ( _calibration.satMin != _calibration.satMax ) {
            if ( logicalValue <= _calibration.satMin )
                return (UInt32)scaledMin;
            if ( logicalValue >= _calibration.satMax )
                return (UInt32)scaledMax;
            
            logicalMin      = _calibration.satMin;
            logicalMax      = _calibration.satMax;
        }
        
        // now check the dead zone
        if (_calibration.dzMin != _calibration.dzMax) {
            SInt64 scaledMid = scaledMin + ((scaledMax - scaledMin) / 2);
            if (logicalValue < _calibration.dzMin) {
                logicalMax = _calibration.dzMin;
                scaledMax = scaledMid;
            } else if ( logicalValue > _calibration.dzMax) {
                logicalMin = _calibration.dzMax;
                scaledMin = scaledMid;
            } else {
                return (UInt32)scaledMid;
            }
        }
        
    } else { // kIOHIDValueScaleTypePhysical
        scaledMin = getPhysicalMin();
        scaledMax = getPhysicalMax();
    }
    
    logicalRange    = logicalMax - logicalMin;
    scaledRange     = scaledMax - scaledMin;
    
    if (logicalRange) {
        returnValue = ((logicalValue - logicalMin) * scaledRange / logicalRange) + scaledMin;
    } else {
        returnValue = logicalValue;
    }
        
    return (UInt32)returnValue;
}

IOFixed IOHIDElementPrivate::getScaledFixedValue(IOHIDValueScaleType type, IOOptionBits options)
{
    if (options & kIOHIDValueOptionsUpdateElementValues) {
        IOReturn status = _owner->updateElementValues(&_cookie, 1);
        if (status) {
            HIDLogError("updateElementValues failed (%lu):%x", (uintptr_t)_cookie, status);
        }
    }
    return getScaledFixedValue (type);
}


IOFixed IOHIDElementPrivate::getScaledFixedValue(IOHIDValueScaleType type)
{
    SInt64  logicalValue    = (SInt32)getValue();
    SInt64  logicalMin      = (SInt32)getLogicalMin();
    SInt64  logicalMax      = (SInt32)getLogicalMax();
    SInt64  logicalRange    = 0;
    SInt64  physicalMin     = (SInt32)getPhysicalMin();
    SInt64  physicalMax     = (SInt32)getPhysicalMax();
    SInt64  physicalRange   = 0;
    IOFixed returnValue     = 0;
    
    if ( type == kIOHIDValueScaleTypeCalibrated ){
        
        if ( _calibration.min != _calibration.max ) {
            physicalMin = _calibration.min;
            physicalMax = _calibration.max;
        } else {
            physicalMin = -1;
            physicalMax =  1;
        }
        
        // check saturation first
        if ( _calibration.satMin != _calibration.satMax ) {
            if ( logicalValue <= _calibration.satMin )
                return (IOFixed) (physicalMin << 16);
            if ( logicalValue >= _calibration.satMax )
                return (IOFixed) (physicalMax << 16);
            
            logicalMin = _calibration.satMin;
            logicalMax = _calibration.satMax;
        }
        
        // now check the dead zone
        if (_calibration.dzMin != _calibration.dzMax) {
            SInt64  physicalMid = physicalMin + ((physicalMax - physicalMin) / 2);
            if (logicalValue < _calibration.dzMin) {
                logicalMax = _calibration.dzMin;
                physicalMax = physicalMid;
            } else if ( logicalValue > _calibration.dzMax) {
                logicalMin = _calibration.dzMax;
                physicalMin = physicalMid;
            } else {
                return (IOFixed) (physicalMid << 16);
            }
        }
        
    }

    UInt32 numExp   = 1;
    UInt32 denomExp = 1;

    if (type == kIOHIDValueScaleTypeExponent) {
        int resExponent  = _unitExponent & 0x0F;

        if (resExponent < 8) {
            for (int i = resExponent; i > 0; i--) {
                numExp *=  10;
            }
        } else {
            for (int i = 0x10 - resExponent; i > 0; i--) {
                denomExp *= 10;
            }
        }
    }

    physicalRange = physicalMax - physicalMin;

    logicalRange  = logicalMax - logicalMin;
    if (!logicalRange) {
        logicalRange = 1;
    }

    SInt64 inValue      = logicalValue - logicalMin;
    SInt64 rangeFactor  = ((physicalRange << 16) / denomExp) * numExp;
    SInt64 rangeOffset  = ((physicalMin << 16) / denomExp) * numExp;

    returnValue = (IOFixed)(((inValue * rangeFactor) / logicalRange) + rangeOffset);
 
    return returnValue;
}

UInt32 IOHIDElementPrivate::getValue(IOOptionBits options) {
    
    UInt32 newValue = 0;
    
    if ((_reportBits * _reportCount) <= 32) {
        if (options & kIOHIDValueOptionsUpdateElementValues) {
            IOReturn status = _owner->updateElementValues(&_cookie, 1);
            if (status) {
                HIDLogError("updateElementValues failed (%lu):%x", (uintptr_t)_cookie, status);
            }
        }
        
        newValue = ( options & kIOHIDValueOptionsFlagPrevious ) ? _previousValue : _elementValue->value[0];

        if ( options & kIOHIDValueOptionsFlagRelativeSimple ) {
            if ( (getFlags() & kIOHIDElementFlagsWrapMask) && newValue == getLogicalMin() && _previousValue == getLogicalMax())
                newValue = 1;
            else if ( (getFlags() & kIOHIDElementFlagsWrapMask) &&  newValue == getLogicalMax() && _previousValue == getLogicalMin())
                newValue = -1;
            else
                newValue -= _previousValue;
        }
    }
    
    return newValue;
}


