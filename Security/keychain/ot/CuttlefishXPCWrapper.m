/*
 * Copyright (c) 2019 Apple Inc. All Rights Reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
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

#import "keychain/ot/CuttlefishXPCWrapper.h"
#import <AppleFeatures/AppleFeatures.h>

@implementation CuttlefishXPCWrapper
- (instancetype) initWithCuttlefishXPCConnection: (id<NSXPCProxyCreating>)cuttlefishXPCConnection
{
    if ((self = [super init])) {
        _cuttlefishXPCConnection = cuttlefishXPCConnection;
    }
    return self;
}

+ (bool)retryable:(NSError *_Nonnull)error
{
    return error.domain == NSCocoaErrorDomain && error.code == NSXPCConnectionInterrupted;
}

enum {NUM_RETRIES = 5};

- (void)pingWithReply:(void (^)(void))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                    }
                    ++i;
                }] pingWithReply:reply];
    } while (retry);
}

- (void)dumpWithSpecificUser:(TPSpecificUser*)specificUser
                       reply:(void (^)(NSDictionary * _Nullable, NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, error);
                    }
                    ++i;
                }] dumpWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)departByDistrustingSelfWithSpecificUser:(TPSpecificUser*)specificUser
                                          reply:(void (^)(NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(error);
                    }
                    ++i;
                }] departByDistrustingSelfWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)distrustPeerIDsWithSpecificUser:(TPSpecificUser*)specificUser
                                peerIDs:(NSSet<NSString*>*)peerIDs
                                  reply:(void (^)(NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(error);
                    }
                    ++i;
                }] distrustPeerIDsWithSpecificUser:specificUser peerIDs:peerIDs reply:reply];
    } while (retry);
}

- (void)trustStatusWithSpecificUser:(TPSpecificUser*)specificUser
                              reply:(void (^)(TrustedPeersHelperEgoPeerStatus *status,
                                              NSError* _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, error);
                    }
                    ++i;
                }] trustStatusWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)resetWithSpecificUser:(TPSpecificUser*)specificUser
                  resetReason:(CuttlefishResetReason)reason
                        reply:(void (^)(NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(error);
                    }
                    ++i;
                }] resetWithSpecificUser:specificUser resetReason:reason reply:reply];
    } while (retry);
}

- (void)localResetWithSpecificUser:(TPSpecificUser*)specificUser
                             reply:(void (^)(NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(error);
                    }
                    ++i;
                }] localResetWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)setAllowedMachineIDsWithSpecificUser:(TPSpecificUser*)specificUser
                           allowedMachineIDs:(NSSet<NSString*> *)allowedMachineIDs
                        honorIDMSListChanges:(BOOL)accountIsDemo
                                       reply:(void (^)(BOOL listDifferences, NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(NO, error);
                    }
                    ++i;
        }] setAllowedMachineIDsWithSpecificUser:specificUser allowedMachineIDs:allowedMachineIDs honorIDMSListChanges:accountIsDemo reply:reply];
    } while (retry);
}

- (void)addAllowedMachineIDsWithSpecificUser:(TPSpecificUser*)specificUser
                                  machineIDs:(NSArray<NSString*> *)machineIDs
                                       reply:(void (^)(NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(error);
                    }
                    ++i;
                }] addAllowedMachineIDsWithSpecificUser:specificUser
                                          machineIDs:machineIDs
                                               reply:reply];
    } while (retry);
}

- (void)removeAllowedMachineIDsWithSpecificUser:(TPSpecificUser*)specificUser
                                     machineIDs:(NSArray<NSString*> *)machineIDs
                                          reply:(void (^)(NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(error);
                    }
                    ++i;
                }] removeAllowedMachineIDsWithSpecificUser:specificUser machineIDs:machineIDs reply:reply];
    } while (retry);
}


- (void)fetchAllowedMachineIDsWithSpecificUser:(TPSpecificUser*)specificUser
                                         reply:(nonnull void (^)(NSSet<NSString *> * _Nullable, NSError * _Nullable))reply {
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, error);
                    }
                    ++i;
                }] fetchAllowedMachineIDsWithSpecificUser:specificUser reply:reply];
    } while (retry);
}


- (void)fetchEgoEpochWithSpecificUser:(TPSpecificUser*)specificUser
                                reply:(void (^)(unsigned long long epoch,
                                                NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(0, error);
                    }
                    ++i;
                }] fetchEgoEpochWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)prepareWithSpecificUser:(TPSpecificUser*)specificUser
                          epoch:(unsigned long long)epoch
                      machineID:(NSString *)machineID
                     bottleSalt:(NSString *)bottleSalt
                       bottleID:(NSString *)bottleID
                        modelID:(NSString *)modelID
                     deviceName:(nullable NSString*)deviceName
                   serialNumber:(NSString *)serialNumber
                      osVersion:(NSString *)osVersion
                  policyVersion:(nullable TPPolicyVersion *)policyVersion
                  policySecrets:(nullable NSDictionary<NSString*,NSData*> *)policySecrets
      syncUserControllableViews:(TPPBPeerStableInfoUserControllableViewStatus)syncUserControllableViews
          secureElementIdentity:(nullable TPPBSecureElementIdentity*)secureElementIdentity
    signingPrivKeyPersistentRef:(nullable NSData *)spkPr
        encPrivKeyPersistentRef:(nullable NSData*)epkPr
                          reply:(void (^)(NSString * _Nullable peerID,
                                          NSData * _Nullable permanentInfo,
                                          NSData * _Nullable permanentInfoSig,
                                          NSData * _Nullable stableInfo,
                                          NSData * _Nullable stableInfoSig,
                                          TPSyncingPolicy* _Nullable syncingPolicy,
                                          NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, nil, nil, nil, nil, error);
                    }
                    ++i;
                }] prepareWithSpecificUser:specificUser
         epoch:epoch
         machineID:machineID
         bottleSalt:bottleSalt
         bottleID:bottleID
         modelID:modelID
         deviceName:deviceName
         serialNumber:serialNumber
         osVersion:osVersion
         policyVersion:policyVersion
         policySecrets:policySecrets
         syncUserControllableViews:syncUserControllableViews
         secureElementIdentity:secureElementIdentity
         setting:setting
         signingPrivKeyPersistentRef:spkPr
         encPrivKeyPersistentRef:epkPr
         reply:reply];
    } while (retry);
}

- (void)establishWithSpecificUser:(TPSpecificUser*)specificUser
                         ckksKeys:(NSArray<CKKSKeychainBackedKeySet*> *)viewKeySets
                        tlkShares:(NSArray<CKKSTLKShare*> *)tlkShares
                  preapprovedKeys:(nullable NSArray<NSData*> *)preapprovedKeys
                            reply:(void (^)(NSString * _Nullable peerID,
                                            NSArray<CKRecord*>* _Nullable keyHierarchyRecords,
                                            TPSyncingPolicy* _Nullable syncingPolicy,
                                            NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, nil, error);
                    }
                    ++i;
                }] establishWithSpecificUser:specificUser ckksKeys:viewKeySets tlkShares:tlkShares preapprovedKeys:preapprovedKeys reply:reply];
    } while (retry);
}

- (void)vouchWithSpecificUser:(TPSpecificUser*)specificUser
                       peerID:(NSString *)peerID
                permanentInfo:(NSData *)permanentInfo
             permanentInfoSig:(NSData *)permanentInfoSig
                   stableInfo:(NSData *)stableInfo
                stableInfoSig:(NSData *)stableInfoSig
                     ckksKeys:(NSArray<CKKSKeychainBackedKeySet*> *)viewKeySets
                        reply:(void (^)(NSData * _Nullable voucher,
                                        NSData * _Nullable voucherSig,
                                     NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, error);
                    }
                    ++i;
                }] vouchWithSpecificUser:specificUser peerID:peerID permanentInfo:permanentInfo permanentInfoSig:permanentInfoSig stableInfo:stableInfo stableInfoSig:stableInfoSig ckksKeys:viewKeySets reply:reply];
    } while (retry);
}


- (void)preflightVouchWithBottleWithSpecificUser:(TPSpecificUser*)specificUser
                                        bottleID:(nonnull NSString *)bottleID
                                           reply:(nonnull void (^)(NSString * _Nullable,
                                                                   TPSyncingPolicy* _Nullable peerSyncingPolicy,
                                                                   BOOL refetchWasNeeded,
                                                                   NSError * _Nullable))reply {
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, false, error);
                    }
                    ++i;
                }] preflightVouchWithBottleWithSpecificUser:specificUser
                                                   bottleID:bottleID
                                                      reply:reply];
    } while (retry);
}

- (void)vouchWithBottleWithSpecificUser:(TPSpecificUser*)specificUser
                               bottleID:(NSString*)bottleID
                                entropy:(NSData*)entropy
                             bottleSalt:(NSString*)bottleSalt
                              tlkShares:(NSArray<CKKSTLKShare*> *)tlkShares
                                  reply:(void (^)(NSData * _Nullable voucher,
                                                  NSData * _Nullable voucherSig,
                                                  NSArray<CKKSTLKShare*>* _Nullable newTLKShares,
                                                  TrustedPeersHelperTLKRecoveryResult* _Nullable tlkRecoveryResults,
                                                  NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, nil, nil, error);
                    }
                    ++i;
                }] vouchWithBottleWithSpecificUser:specificUser bottleID:bottleID entropy:entropy bottleSalt:bottleSalt tlkShares:tlkShares reply:reply];
    } while (retry);
}

- (void)preflightVouchWithRecoveryKeyWithSpecificUser:(TPSpecificUser*)specificUser
                                          recoveryKey:(NSString*)recoveryKey
                                                 salt:(NSString*)salt
                                                reply:(nonnull void (^)(NSString * _Nullable,
                                                                        TPSyncingPolicy* _Nullable peerSyncingPolicy,
                                                                        NSError * _Nullable))reply {
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, error);
                    }
                    ++i;
                }] preflightVouchWithRecoveryKeyWithSpecificUser:specificUser
                                                     recoveryKey:recoveryKey
                                                            salt:salt
                                                           reply:reply];
    } while (retry);
}

- (void)preflightVouchWithCustodianRecoveryKeyWithSpecificUser:(TPSpecificUser*)specificUser
                                                           crk:(TrustedPeersHelperCustodianRecoveryKey*)crk
                                                         reply:(void (^)(NSString* _Nullable recoveryKeyID,
                                                                         TPSyncingPolicy* _Nullable syncingPolicy,
                                                                         NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, error);
                    }
                    ++i;
                }] preflightVouchWithCustodianRecoveryKeyWithSpecificUser:specificUser
                                                                      crk:crk
                                                                    reply:reply];
    } while (retry);
}

- (void)vouchWithRecoveryKeyWithSpecificUser:(TPSpecificUser*)specificUser
                                 recoveryKey:(NSString*)recoveryKey
                                        salt:(NSString*)salt
                                   tlkShares:(NSArray<CKKSTLKShare*> *)tlkShares
                                       reply:(void (^)(NSData * _Nullable voucher,
                                                       NSData * _Nullable voucherSig,
                                                       NSArray<CKKSTLKShare*>* _Nullable newTLKShares,
                                                       TrustedPeersHelperTLKRecoveryResult* _Nullable tlkRecoveryResults,
                                                       NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, nil, nil, error);
                    }
                    ++i;
                }] vouchWithRecoveryKeyWithSpecificUser:specificUser recoveryKey:recoveryKey salt:salt tlkShares:tlkShares reply:reply];
    } while (retry);
}

- (void)vouchWithCustodianRecoveryKeyWithSpecificUser:(TPSpecificUser*)specificUser
                                                  crk:(TrustedPeersHelperCustodianRecoveryKey*)crk
                                            tlkShares:(NSArray<CKKSTLKShare*> *)tlkShares
                                                reply:(void (^)(NSData * _Nullable voucher,
                                                                NSData * _Nullable voucherSig,
                                                                NSArray<CKKSTLKShare*>* _Nullable newTLKShares,
                                                                TrustedPeersHelperTLKRecoveryResult* _Nullable tlkRecoveryResults,
                                                                NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, nil, nil, error);
                    }
                    ++i;
                }] vouchWithCustodianRecoveryKeyWithSpecificUser:specificUser crk:crk tlkShares:tlkShares reply:reply];
    } while (retry);
}

- (void)joinWithSpecificUser:(TPSpecificUser*)specificUser
                 voucherData:(NSData *)voucherData
                  voucherSig:(NSData *)voucherSig
                    ckksKeys:(NSArray<CKKSKeychainBackedKeySet*> *)viewKeySets
                   tlkShares:(NSArray<CKKSTLKShare*> *)tlkShares
             preapprovedKeys:(nullable NSArray<NSData*> *)preapprovedKeys
                       reply:(void (^)(NSString * _Nullable peerID,
                                       NSArray<CKRecord*>* _Nullable keyHierarchyRecords,
                                       TPSyncingPolicy* _Nullable syncingPolicy,
                                    NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, nil, error);
                    }
                    ++i;
                }] joinWithSpecificUser:specificUser voucherData:voucherData voucherSig:voucherSig ckksKeys:viewKeySets tlkShares:tlkShares preapprovedKeys:preapprovedKeys reply:reply];
    } while (retry);
}

- (void)preflightPreapprovedJoinWithSpecificUser:(TPSpecificUser*)specificUser
                                 preapprovedKeys:(nullable NSArray<NSData*> *)preapprovedKeys
                                           reply:(void (^)(BOOL launchOkay,
                                                           NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(NO, error);
                    }
                    ++i;
        }] preflightPreapprovedJoinWithSpecificUser:specificUser preapprovedKeys:preapprovedKeys reply:reply];
    } while (retry);
}

- (void)attemptPreapprovedJoinWithSpecificUser:(TPSpecificUser*)specificUser
                                      ckksKeys:(NSArray<CKKSKeychainBackedKeySet*> *)ckksKeys
                                     tlkShares:(NSArray<CKKSTLKShare*> *)tlkShares
                               preapprovedKeys:(nullable NSArray<NSData*> *)preapprovedKeys
                                         reply:(void (^)(NSString * _Nullable peerID,
                                                         NSArray<CKRecord*>* _Nullable keyHierarchyRecords,
                                                         TPSyncingPolicy* _Nullable syncingPolicy,
                                                         NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, nil, error);
                    }
                    ++i;
        }] attemptPreapprovedJoinWithSpecificUser:specificUser
                                      ckksKeys:ckksKeys
                                     tlkShares:tlkShares
                               preapprovedKeys:preapprovedKeys
                                         reply:reply];
    } while (retry);
}

- (void)updateWithSpecificUser:(TPSpecificUser*)specificUser
                  forceRefetch:(BOOL)forceRefetch
                    deviceName:(nullable NSString *)deviceName
                  serialNumber:(nullable NSString *)serialNumber
                     osVersion:(nullable NSString *)osVersion
                 policyVersion:(nullable NSNumber *)policyVersion
                 policySecrets:(nullable NSDictionary<NSString*,NSData*> *)policySecrets
     syncUserControllableViews:(nullable NSNumber *)syncUserControllableViews
         secureElementIdentity:(nullable TrustedPeersHelperIntendedTPPBSecureElementIdentity*)secureElementIdentity
                         reply:(void (^)(TrustedPeersHelperPeerState* _Nullable peerState, TPSyncingPolicy* _Nullable policy, NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, error);
                    }
                    ++i;
                }] updateWithSpecificUser:specificUser
                          forceRefetch:forceRefetch
                            deviceName:deviceName
                          serialNumber:serialNumber
                             osVersion:osVersion
                         policyVersion:policyVersion
                         policySecrets:policySecrets
             syncUserControllableViews:syncUserControllableViews
                 secureElementIdentity:secureElementIdentity
                                 reply:reply];
    } while (retry);
}

- (void)setPreapprovedKeysWithSpecificUser:(TPSpecificUser*)specificUser
                           preapprovedKeys:(NSArray<NSData*> *)preapprovedKeys
                                     reply:(void (^)(TrustedPeersHelperPeerState* _Nullable peerState, NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, error);
                    }
                    ++i;
                }] setPreapprovedKeysWithSpecificUser:specificUser preapprovedKeys:preapprovedKeys reply:reply];
    } while (retry);
}

- (void)updateTLKsWithSpecificUser:(TPSpecificUser*)specificUser
                          ckksKeys:(NSArray<CKKSKeychainBackedKeySet*> *)ckksKeys
                         tlkShares:(NSArray<CKKSTLKShare*> *)tlkShares
                             reply:(void (^)(NSArray<CKRecord*>* _Nullable keyHierarchyRecords, NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, error);
                    }
                    ++i;
                }] updateTLKsWithSpecificUser:specificUser ckksKeys:ckksKeys tlkShares:tlkShares reply:reply];
    } while (retry);
}

- (void)fetchViableBottlesWithSpecificUser:(TPSpecificUser*)specificUser
                                    source:(OTEscrowRecordFetchSource)source
                                     reply:(void (^)(NSArray<NSString*>* _Nullable sortedBottleIDs, NSArray<NSString*>* _Nullable sortedPartialBottleIDs, NSError* _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, error);
                    }
                    ++i;
        }] fetchViableBottlesWithSpecificUser:specificUser source:source reply:reply];
    } while (retry);
}
   
- (void)fetchEscrowContentsWithSpecificUser:(TPSpecificUser*)specificUser
                                      reply:(void (^)(NSData* _Nullable entropy,
                                                      NSString* _Nullable bottleID,
                                                      NSData* _Nullable signingPublicKey,
                                                      NSError* _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, nil, error);
                    }
                    ++i;
                }] fetchEscrowContentsWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)fetchPolicyDocumentsWithSpecificUser:(TPSpecificUser*)specificUser
                                    versions:(NSSet<TPPolicyVersion*>*)versions
                                       reply:(void (^)(NSDictionary<TPPolicyVersion*, NSData*>* _Nullable entries,
                                                       NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, error);
                    }
                    ++i;
                }] fetchPolicyDocumentsWithSpecificUser:specificUser versions:versions reply:reply];
    } while (retry);
}

- (void)fetchCurrentPolicyWithSpecificUser:(TPSpecificUser*)specificUser
                           modelIDOverride:(NSString* _Nullable)modelIDOverride
                        isInheritedAccount:(BOOL)isInheritedAccount
                                     reply:(void (^)(TPSyncingPolicy* _Nullable syncingPolicy,
                                                     TPPBPeerStableInfoUserControllableViewStatus userControllableViewStatusOfPeers,
                                                     NSError * _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil,
                              TPPBPeerStableInfoUserControllableViewStatus_UNKNOWN,
                              error);
                    }
                    ++i;
        }] fetchCurrentPolicyWithSpecificUser:specificUser modelIDOverride:modelIDOverride isInheritedAccount:isInheritedAccount reply:reply];
    } while (retry);
}


- (void)validatePeersWithSpecificUser:(TPSpecificUser*)specificUser
                                reply:(void (^)(NSDictionary * _Nullable, NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, error);
                    }
                    ++i;
                }] validatePeersWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)fetchTrustStateWithSpecificUser:(TPSpecificUser*)specificUser
                                  reply:(void (^)(TrustedPeersHelperPeerState* _Nullable selfPeerState,
                                                  NSArray<TrustedPeersHelperPeer*>* _Nullable trustedPeers,
                                                  NSError* _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, error);
                    }
                    ++i;
                }] fetchTrustStateWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)setRecoveryKeyWithSpecificUser:(TPSpecificUser*)specificUser
                           recoveryKey:(NSString *)recoveryKey
                                  salt:(NSString *)salt
                              ckksKeys:(NSArray<CKKSKeychainBackedKeySet*> *)ckksKeys
                                 reply:(void (^)(NSArray<CKRecord*>* _Nullable keyHierarchyRecords,
                                                 NSError* _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, error);
                    }
                    ++i;
                }] setRecoveryKeyWithSpecificUser:specificUser recoveryKey:recoveryKey salt:salt ckksKeys:ckksKeys reply:reply];
    } while (retry);
}

- (void)createCustodianRecoveryKeyWithSpecificUser:(TPSpecificUser*)specificUser
                                       recoveryKey:(NSString *)recoveryString
                                              salt:(NSString *)salt
                                          ckksKeys:(NSArray<CKKSKeychainBackedKeySet*> *)ckksKeys
                                              uuid:(NSUUID *)uuid
                                              kind:(TPPBCustodianRecoveryKey_Kind)kind
                                             reply:(void (^)(NSArray<CKRecord*>* _Nullable keyHierarchyRecords,
                                                             TrustedPeersHelperCustodianRecoveryKey* _Nullable crk,
                                                             NSError* _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(nil, nil, error);
                    }
                    ++i;
                }] createCustodianRecoveryKeyWithSpecificUser:specificUser recoveryKey:recoveryString salt:salt ckksKeys:ckksKeys uuid:uuid kind:kind reply:reply];
    } while (retry);
}

- (void)removeCustodianRecoveryKeyWithSpecificUser:(TPSpecificUser*)specificUser
                                              uuid:(NSUUID *)uuid
                                             reply:(void (^)(NSError* _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(error);
                    }
                    ++i;
                }] removeCustodianRecoveryKeyWithSpecificUser:specificUser uuid:uuid reply:reply];
    } while (retry);
}

- (void)reportHealthWithSpecificUser:(TPSpecificUser*)specificUser
                   stateMachineState:(NSString *)state
                          trustState:(NSString *)trustState
                               reply:(void (^)(NSError* _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(error);
                    }
                    ++i;
                }] reportHealthWithSpecificUser:specificUser stateMachineState:state trustState:trustState reply:reply];
    } while (retry);
}

- (void)pushHealthInquiryWithSpecificUser:(TPSpecificUser*)specificUser
                                    reply:(void (^)(NSError* _Nullable error))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(error);
                    }
                    ++i;
                }] pushHealthInquiryWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)requestHealthCheckWithSpecificUser:(TPSpecificUser*)specificUser
                       requiresEscrowCheck:(BOOL)requiresEscrowCheck
                          knownFederations:(NSArray<NSString *> *)knownFederations
                                     reply:(void (^)(BOOL postRepairCFU, BOOL postEscrowCFU, BOOL resetOctagon, BOOL leaveTrust, OTEscrowMoveRequestContext *moveRequest, NSError* _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
                    if (i < NUM_RETRIES && [self.class retryable:error]) {
                        secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                        retry = true;
                    } else {
                        secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                        reply(NO, NO, NO, NO, nil, error);
                    }
                    ++i;
        }] requestHealthCheckWithSpecificUser:specificUser requiresEscrowCheck:requiresEscrowCheck knownFederations:knownFederations reply:reply];
    } while (retry);
}

- (void)getSupportAppInfoWithSpecificUser:(TPSpecificUser*)specificUser
                                    reply:(void (^)(NSData * _Nullable, NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
            if (i < NUM_RETRIES && [self.class retryable:error]) {
                secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                retry = true;
            } else {
                secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                reply(nil, error);
            }
            ++i;
        }] getSupportAppInfoWithSpecificUser:specificUser reply:reply];
    } while (retry);

}

- (void)fetchViableEscrowRecordsWithSpecificUser:(TPSpecificUser *)specificUser
                                          source:(OTEscrowRecordFetchSource)source
                                           reply:(void (^)(NSArray<NSData*>* _Nullable,
                                                           NSError* _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError* _Nonnull error) {
            if (i < NUM_RETRIES && [self.class retryable:error]) {
                secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                retry = true;
            } else {
                secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                reply(nil, error);
            }
            ++i;
        }] fetchViableEscrowRecordsWithSpecificUser:specificUser source:source reply:reply];
    } while (retry);
}

- (void)removeEscrowCacheWithSpecificUser:(TPSpecificUser*)specificUser
                                    reply:(nonnull void (^)(NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
            if (i < NUM_RETRIES && [self.class retryable:error]) {
                secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                retry = true;
            } else {
                secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                reply(error);
            }
            ++i;
        }] removeEscrowCacheWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)resetAccountCDPContentsWithSpecificUser:(TPSpecificUser*)specificUser
                                          reply:(nonnull void (^)(NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
            if (i < NUM_RETRIES && [self.class retryable:error]) {
                secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                retry = true;
            } else {
                secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                reply(error);
            }
            ++i;
        }] resetAccountCDPContentsWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)fetchRecoverableTLKSharesWithSpecificUser:(TPSpecificUser*)specificUser
                                           peerID:(NSString * _Nullable)peerID
                                            reply:(nonnull void (^)(NSArray<CKRecord *> * _Nullable,
                                                                    NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
            if (i < NUM_RETRIES && [self.class retryable:error]) {
                secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                retry = true;
            } else {
                secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                reply(nil, error);
            }
            ++i;
        }] fetchRecoverableTLKSharesWithSpecificUser:specificUser peerID:peerID reply:reply];
    } while (retry);
}

- (void)fetchAccountSettingsWithSpecificUser:(TPSpecificUser*)specificUser
                                       reply:(nonnull void (^)(NSDictionary<NSString*, TPPBPeerStableInfoSetting *> * _Nullable,
                                                               NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
            if (i < NUM_RETRIES && [self.class retryable:error]) {
                secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                retry = true;
            } else {
                secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                reply(nil, error);
            }
            ++i;
        }] fetchAccountSettingsWithSpecificUser:specificUser reply:reply];
    } while (retry);
}

- (void)prepareInheritancePeerWithSpecificUser:(TPSpecificUser*)specificUser
                                         epoch:(unsigned long long)epoch
                                     machineID:(nonnull NSString *)machineID
                                    bottleSalt:(nonnull NSString *)bottleSalt
                                      bottleID:(nonnull NSString *)bottleID
                                       modelID:(nonnull NSString *)modelID
                                    deviceName:(nullable NSString *)deviceName
                                  serialNumber:(nullable NSString *)serialNumber
                                     osVersion:(nonnull NSString *)osVersion
                                 policyVersion:(nullable TPPolicyVersion *)policyVersion
                                 policySecrets:(nullable NSDictionary<NSString *,NSData *> *)policySecrets syncUserControllableViews:(TPPBPeerStableInfoUserControllableViewStatus)syncUserControllableViews
                         secureElementIdentity:(nullable TPPBSecureElementIdentity *)secureElementIdentity
                   signingPrivKeyPersistentRef:(nullable NSData *)spkPr
                       encPrivKeyPersistentRef:(nullable NSData *)epkPr
                                           crk:(nonnull TrustedPeersHelperCustodianRecoveryKey *)crk
                                         reply:(nonnull void (^)(NSString * _Nullable, NSData * _Nullable, NSData * _Nullable, NSData * _Nullable, NSData * _Nullable, TPSyncingPolicy * _Nullable, NSString * _Nullable,                                                       NSArray<CKRecord*>* _Nullable keyHierarchyRecords, NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
            if (i < NUM_RETRIES && [self.class retryable:error]) {
                secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                retry = true;
            } else {
                secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                reply(nil, nil, nil, nil, nil, nil, nil, nil, error);
            }
            ++i;
        }] prepareInheritancePeerWithSpecificUser:specificUser
         epoch:epoch
         machineID:machineID
         bottleSalt:bottleSalt
         bottleID:bottleID
         modelID:modelID
         deviceName:deviceName
         serialNumber:serialNumber
         osVersion:osVersion
         policyVersion:policyVersion
         policySecrets:policySecrets
         syncUserControllableViews:syncUserControllableViews
         secureElementIdentity:secureElementIdentity
         signingPrivKeyPersistentRef:spkPr
         encPrivKeyPersistentRef:epkPr
         crk:crk
         reply:reply];
    } while (retry);
}

- (void)recoverTLKSharesForInheritorWithSpecificUser:(TPSpecificUser*)specificUser
                                                 crk:(nonnull TrustedPeersHelperCustodianRecoveryKey *)crk
                                           tlkShares:(nonnull NSArray<CKKSTLKShare *> *)tlkShares
                                               reply:(nonnull void (^)(NSArray<CKKSTLKShare *> * _Nullable,
                                                                       TrustedPeersHelperTLKRecoveryResult* _Nullable tlkRecoveryResults,
                                                                       NSError * _Nullable))reply
{
    __block int i = 0;
    __block bool retry;
    do {
        retry = false;
        [[self.cuttlefishXPCConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError *_Nonnull error) {
            if (i < NUM_RETRIES && [self.class retryable:error]) {
                secnotice("octagon", "retrying cuttlefish XPC, (%d, %@)", i, error);
                retry = true;
            } else {
                secerror("octagon: Can't talk with TrustedPeersHelper: %@", error);
                reply(nil, nil, error);
            }
            ++i;
        }] recoverTLKSharesForInheritorWithSpecificUser:specificUser crk:crk tlkShares:tlkShares reply:reply];
    } while (retry);
}

@end
