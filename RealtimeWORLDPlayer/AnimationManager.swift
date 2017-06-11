//
//  AnimationManager.swift
//  VowelsMorphing
//
//  Created by Yusuke Watanabe on 2017/06/11.
//  Copyright © 2017年 Yusuke Watanabe. All rights reserved.
//

import Foundation
import UIKit
import SpriteKit


class AnimationManager{
    //--------エフェクト系の設定--------
    let MaleEffectPath = Bundle.main.path(forResource:"particle",ofType:"sks")
    
    var particle = SKEmitterNode()      //タップ位置に表示するエフェクト
    let ParticleMoveSpeed = 0.07        //追尾particle速度
    
    init(scene:SKScene,frame:CGRect){
        loadEffectData(scene: scene, frame: frame)
    }
    
    //エフェクトデータの読み込み
    func loadEffectData(scene:SKScene,frame:CGRect){
        let pos :CGPoint = CGPoint(x:frame.size.width/2,y:frame.size.height/2)
        
        particle = NSKeyedUnarchiver.unarchiveObject(withFile: MaleEffectPath!) as! SKEmitterNode
        particle.name = "particle"
        particle.targetNode = scene
        particle.position = pos
        
        scene.addChild(particle)
    }
    func moveParticle(x:CGFloat,y:CGFloat,moveSpeed:Double){
        let XMoveAction = SKAction.moveTo(x: x, duration: moveSpeed)
        let YMoveAction = SKAction.moveTo(y: y, duration: moveSpeed)
        XMoveAction.timingMode = SKActionTimingMode.easeInEaseOut
        YMoveAction.timingMode = SKActionTimingMode.easeInEaseOut
        particle.run(XMoveAction)
        particle.run(YMoveAction)
    }
}
