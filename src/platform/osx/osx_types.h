

#ifndef __OSX_TYPES_H__
#define __OSX_TYPES_H__


typedef CGRect NSRect;
typedef CGPoint NSPoint;
typedef CGSize NSSize;

typedef unsigned long NSUInteger;
typedef long NSInteger;

/* NSWindowStyleMask */
#define NSWindowStyleMaskBorderless              (0)
#define NSWindowStyleMaskTitled                  (1 << 0)
#define NSWindowStyleMaskClosable                (1 << 1)
#define NSWindowStyleMaskMiniaturizable          (1 << 2)
#define NSWindowStyleMaskResizable               (1 << 3)
#define NSWindowStyleMaskUtilityWindow           (1 << 4)
#define NSWindowStyleMaskDocModalWindow          (1 << 6)
#define NSWindowStyleMaskNonactivatingPanel      (1 << 7)
#define NSWindowStyleMaskTexturedBackground      (1 << 8) /* deprecated */
#define NSWindowStyleMaskUnifiedTitleAndToolbar  (1 << 12)
#define NSWindowStyleMaskHUDWindow               (1 << 13)
#define NSWindowStyleMaskFullScreen              (1 << 14)
#define NSWindowStyleMaskFullSizeContentView     (1 << 15)

/* NSEventType */
#define NSEventTypeLeftMouseDown              (1)
#define NSEventTypeLeftMouseUp                (2)
#define NSEventTypeRightMouseDown             (3)
#define NSEventTypeRightMouseUp               (4)
#define NSEventTypeMouseMoved                 (5)
#define NSEventTypeLeftMouseDragged           (6)
#define NSEventTypeRightMouseDragged          (7)
#define NSEventTypeMouseEntered               (8)
#define NSEventTypeMouseExited                (9)
#define NSEventTypeKeyDown                   (10)
#define NSEventTypeKeyUp                     (11)
#define NSEventTypeFlagsChanged              (12)
#define NSEventTypeAppKitDefined             (13)
#define NSEventTypeSystemDefined             (14)
#define NSEventTypeApplicationDefined        (15)
#define NSEventTypePeriodic                  (16)
#define NSEventTypeCursorUpdate              (17)
#define NSEventTypeScrollWheel               (22)
#define NSEventTypeTabletPoint               (23)
#define NSEventTypeTabletProximity           (24)
#define NSEventTypeOtherMouseDown            (25)
#define NSEventTypeOtherMouseUp              (26)
#define NSEventTypeOtherMouseDragged         (27)

/* The following event types are available on some hardware on 10.5.2 and later */
/*
#define NSEventTypeGesture       API_AVAILABLE(macos(10.5)) => 29,
#define NSEventTypeMagnify       API_AVAILABLE(macos(10.5)) => 30,
#define NSEventTypeSwipe         API_AVAILABLE(macos(10.5)) => 31,
#define NSEventTypeRotate        API_AVAILABLE(macos(10.5)) => 18,
#define NSEventTypeBeginGesture  API_AVAILABLE(macos(10.5)) => 19,
#define NSEventTypeEndGesture    API_AVAILABLE(macos(10.5)) => 20,
#define NSEventTypeSmartMagnify  API_AVAILABLE(macos(10.8)) => 32,
#define NSEventTypeQuickLook     API_AVAILABLE(macos(10.8)) => 33,
#define NSEventTypePressure      API_AVAILABLE(macos(10.10.3)) => 34,
#define NSEventTypeDirectTouch   API_AVAILABLE(macos(10.10)) => 37,
#define NSEventTypeChangeMode    API_AVAILABLE(macos(10.15)) => 38,
 */


/* NSEventMask */
#define NSEventMaskLeftMouseDown         (1ULL << NSEventTypeLeftMouseDown)
#define NSEventMaskLeftMouseUp           (1ULL << NSEventTypeLeftMouseUp)
#define NSEventMaskRightMouseDown        (1ULL << NSEventTypeRightMouseDown)
#define NSEventMaskRightMouseUp          (1ULL << NSEventTypeRightMouseUp)
#define NSEventMaskMouseMoved            (1ULL << NSEventTypeMouseMoved)
#define NSEventMaskLeftMouseDragged      (1ULL << NSEventTypeLeftMouseDragged)
#define NSEventMaskRightMouseDragged     (1ULL << NSEventTypeRightMouseDragged)
#define NSEventMaskMouseEntered          (1ULL << NSEventTypeMouseEntered)
#define NSEventMaskMouseExited           (1ULL << NSEventTypeMouseExited)
#define NSEventMaskKeyDown               (1ULL << NSEventTypeKeyDown)
#define NSEventMaskKeyUp                 (1ULL << NSEventTypeKeyUp)
#define NSEventMaskFlagsChanged          (1ULL << NSEventTypeFlagsChanged)
#define NSEventMaskAppKitDefined         (1ULL << NSEventTypeAppKitDefined)
#define NSEventMaskSystemDefined         (1ULL << NSEventTypeSystemDefined)
#define NSEventMaskApplicationDefined    (1ULL << NSEventTypeApplicationDefined)
#define NSEventMaskPeriodic              (1ULL << NSEventTypePeriodic)
#define NSEventMaskCursorUpdate          (1ULL << NSEventTypeCursorUpdate)
#define NSEventMaskScrollWheel           (1ULL << NSEventTypeScrollWheel)
#define NSEventMaskTabletPoint           (1ULL << NSEventTypeTabletPoint)
#define NSEventMaskTabletProximity       (1ULL << NSEventTypeTabletProximity)
#define NSEventMaskOtherMouseDown        (1ULL << NSEventTypeOtherMouseDown)
#define NSEventMaskOtherMouseUp          (1ULL << NSEventTypeOtherMouseUp)
#define NSEventMaskOtherMouseDragged     (1ULL << NSEventTypeOtherMouseDragged)

/* NSEventModifierFlag */
#define NSEventModifierFlagCapsLock      (1 << 16) // Set if Caps Lock key is pressed.
#define NSEventModifierFlagShift         (1 << 17) // Set if Shift key is pressed.
#define NSEventModifierFlagControl       (1 << 18) // Set if Control key is pressed.
#define NSEventModifierFlagOption        (1 << 19) // Set if Option or Alternate key is pressed.
#define NSEventModifierFlagCommand       (1 << 20) // Set if Command key is pressed.
#define NSEventModifierFlagNumericPad    (1 << 21) // Set if any key in the numeric keypad is pressed.
#define NSEventModifierFlagHelp          (1 << 22) // Set if the Help key is pressed.
#define NSEventModifierFlagFunction      (1 << 23) // Set if any function key is pressed.




#endif /* __OSX_TYPES_H__ */