const path=require("path")
const grpc = require('@grpc/grpc-js')
const protoLoader = require('@grpc/proto-loader')

const PROTO_PATH=path.join(__dirname,"../Protos/message.proto")
console.log("Message路径"+PROTO_PATH)

/**
 * 同步加载指定的proto文件
 * keepCase: true ——保留大小写；longs: String ——长整数按string读；enums: String ——枚举按string读写
 * defaults: true ——未指定的按默认处理；oneofs: true ——启用protobuf中的oneof 修饰词，类似联合体
 */
const packageDefinition = protoLoader.loadSync(PROTO_PATH, { keepCase: true, longs: String, enums: String, defaults: true, oneofs: true })
const protoDescriptor = grpc.loadPackageDefinition(packageDefinition)
const message_proto = protoDescriptor.message //获取指定的message结构体

module.exports = message_proto //导出该对象