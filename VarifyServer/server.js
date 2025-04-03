const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const const_module = require('./glob')
const { v4: uuidv4 } = require('uuid');
const emailModule = require('./email');
const redis_module = require('./redis')
const config_mgr=require('./config')

/**
 * 查询或生成验证码
 * @email {*}
 */
async function generateAndStoreCode(email) {
    try {
        // 1. 查询Redis中的验证码
        const redisKey = const_module.code_prefix + email;
        let existingCode = await redis_module.GetRedis(redisKey);
        
        // 2. 存在则直接返回
        if (existingCode !== null) return existingCode;

        // 3. 生成新验证码（带截断逻辑）
        let newCode = uuidv4().substring(0, 4);
        
        // 4. 存储到Redis（带10分钟过期）
        const storeSuccess = await redis_module.SetRedisExpire(
            redisKey, 
            newCode, 
            180
        );

        // 5. 处理存储结果
        return storeSuccess ? newCode : { error: const_module.Errors.RedisErr };
    } catch (error) {
        console.error("Code generation error:", error);
        return { error: const_module.Errors.Exception };
    }
}

/**
 * GetVarifyCode grpc响应获取验证码的服务
 * @param {*} call 为grpc请求 
 * @param {*} callback 为grpc回调
 */
async function GetVarifyCodeFunc(call, callback) {
    console.log("email is ", call.request.email)
    try{
       // 调用封装函数获取验证码
       const codeResult = await generateAndStoreCode(call.request.email);
        
       // 错误处理分支
       if (codeResult.error) {
           return callback(null, {
               email: call.request.email,
               error: codeResult.error
           });
       }

        // 邮件发送逻辑
        const mailOptions = {
            from: config_mgr.email_user,
            to: call.request.email,
            subject: '验证码',
            text: `您的验证码为${codeResult}，请三分钟内完成注册`
        };
    
        const sendSuccess = await emailModule.SendMail(mailOptions);
        
        callback(null, {
            email: call.request.email,
            error: sendSuccess ? 
                const_module.Errors.Success : 
                const_module.Errors.EmailSendErr
        });
        
    }catch(error){
        console.log("获取验证码错误 ：", error)

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