const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const const_module = require('./glob')
const { v4: uuidv4 } = require('uuid');
const emailModule = require('./email');
//const redis_module = require('./redis')

/**
 * GetVarifyCode grpc响应获取验证码的服务
 * @param {*} call 为grpc请求 
 * @param {*} callback 为grpc回调
 */
async function GetVarifyCodeFunc(call, callback) {
    console.log("email is ", call.request.email)
    try{
        uniqueId = uuidv4();//生成个码
        console.log("uniqueId is ", uniqueId)
        let text_str =  '您的验证码为'+ uniqueId +'请三分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: 'gosei_chen@qq.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };
    
        let send_res = await emailModule.SendMail(mailOptions);//等待 Promise 调用完成
        console.log("send res is ", send_res)

        callback(null, { 
            email:  call.request.email,
            error:const_module.Errors.Success
        }); 
        
 
    }catch(error){
        console.log("catch error is ", error)

        callback(null, { 
            email:  call.request.email,
            error:const_module.Errors.Exception
        }); 
    }
     
}

function main() {
    var server = new grpc.Server()

    /**
     * VarifyService.service 在proto文件中存在服务名VarifyService，而服务名.service 就是获取其中的服务体
     * { GetVarifyCode: GetVarifyCode } 表示 服务名:接口。也就是以GetVarifyCode服务调用GetVarifyCodeFunc接口
     * grpc.ServerCredentials.createInsecure()相当于做了个安全校验
     */
    server.addService(message_proto.VarifyService.service, { GetVarifyCode: GetVarifyCodeFunc })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        server.start()
        console.log('grpc server started')        
    })
}

main()