const config_module = require('./config')
const Redis = require("ioredis");

// 创建Redis客户端实例
const RedisCli = new Redis({
    host: config_module.redis_host,       // Redis服务器主机名
    port: config_module.redis_port,        // Redis服务器端口号
    password: config_module.redis_passwd, // Redis密码
  });

/**
 * 监听错误信息
 */
RedisCli.on("error", function (err) {
    console.log("RedisCli 连接错误","IPV4:",config_module.redis_host," 端口：",port," 密码：",password);
    RedisCli.quit();
  });

/** async function 表示异步函数
 * 根据key获取value
 * @param {*} key 
 * @returns 
 */
async function GetRedis(key) {
    
    try{
        const result = await RedisCli.get(key)
        if(result === null){
          console.log('result:','<'+result+'>', '未找到该键...')
          return null
        }
        console.log('Result:','<'+result+'>','成功获取到键!...');
        return result
    }catch(error){
        console.log('GetRedis 错误 为：', error);
        return null
    }

  }

/**
 * 根据key查询redis中是否存在key
 * @param {*} key 
 * @returns 
 */
async function QueryRedis(key) {
    try{
        const result = await RedisCli.exists(key)
        //  判断该值是否为空 如果为空返回null
        if (result === 0) {
          console.log('result:<','<'+result+'>','该键为空...');
          return null
        }
        console.log('Result:','<'+result+'>','为键',key,'返回值!...');
        return result
    }catch(error){
        console.log('QueryRedis 错误为', error);
        return null
    }

  }

/**
 * 设置key和value，并过期时间
 * @param {*} key 
 * @param {*} value 
 * @param {*} n秒之后过期 
 * @returns 
 */
async function SetRedisExpire(key,value, exptime){
    try{
        // 设置键和值
        await RedisCli.set(key,value)
        // 设置过期时间（以秒为单位）
        await RedisCli.expire(key, exptime);
        return true;
    }catch(error){
        console.log('SetRedisExpire 错误：', error);
        return false;
    }
}

/**
 * 退出函数
 */
function Quit(){
    RedisCli.quit();
}

module.exports = {GetRedis, QueryRedis, Quit, SetRedisExpire,}