/**************************************************************************
 * 邮箱验证码借用邮箱的smtp服务，用于系统之间的邮件信息传递和提供来信通知       *
 *      将生成的验证码发送给指定邮箱，js直接采用nodemailer库 更加方便         *
 *************************************************************************/

const nodemailer = require('nodemailer');
const config_module = require("./config")

/**
 * 创建发送邮件的代理
 */
let transport = nodemailer.createTransport({
    host: 'smtp.qq.com',
    port: 465,
    secure: true,
    auth: {
        user: config_module.email_user, // 发送方邮箱地址
        pass: config_module.email_pass // 邮箱授权码或者密码
    },
	tls: {
    rejectUnauthorized: false // 禁用证书验证[3,5](@ref)
  }
});

/**
 * 发送邮件的函数
 * @param {*} mailOptions_ 发送邮件的参数
 * @returns 
 */
function SendMail(mailOptions_){
    /**
     * Promis类似于C++中的feature,async是一种异步机制
     *   -其绑定函数两个参数有一个调用即视为调用了
     */
    return new Promise(function(resolve, reject){
        transport.sendMail(mailOptions_, function(error, info)
        {
            if (error) {
                console.log(error);
                reject(error);
            } else {
                console.log('邮件已成功发送：' + info.response);
                resolve(info.response)
            }
        });
    })
   
}

module.exports.SendMail = SendMail