import asyncio
import websockets
import json
from datetime import datetime


async def handler(websocket):
    # 打印客户端 IP 地址
    client_ip = websocket.remote_address[0]
    print(f"Client connected from IP: {client_ip}")

    # Step 1: 发送初始化消息
    init_msg = {
        "type": "init",
        "client_id": "7f0000010fa300000003",
        "msg": "Connect Is Success - AppChat",
        "timestamp": "1747030450"
    }
    await websocket.send(json.dumps(init_msg))
    print("Sent init message")

    # Step 2: 创建接收消息和发送命令的任务
    receive_task = asyncio.create_task(receive_messages(websocket))
    send_task = asyncio.create_task(send_commands(websocket))

    # 等待任务结束（连接断开时）
    await asyncio.gather(receive_task, send_task)


async def receive_messages(websocket):
    try:
        async for message in websocket:
            try:
                data = json.loads(message)
                msg_type = data.get('type')

                # 处理心跳请求
                if msg_type == 'heartbeat':
                    hb_ack = {
                        "type": "heartbeat_ack",
                        "data": {
                            "timestamp": datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
                        }
                    }
                    await websocket.send(json.dumps(hb_ack))
                    # print("Sent heartbeat_ack")

                # 打印识别结果
                elif msg_type == 'result_recognition':
                    print("Received recognition result:")
                    print(json.dumps(data, indent=2))

                # 新增：处理拆装操作结果
                elif msg_type == 'result_assembly':
                    torque_value = data.get('data', {}).get('torque_value')
                    print(f"Received assembly result - Torque: {torque_value} N·m")
                    print(json.dumps(data, indent=2))

            except json.JSONDecodeError:
                print("Received invalid JSON message.")
    except websockets.exceptions.ConnectionClosed:
        print("Client disconnected.")


async def send_commands(websocket):
    loop = asyncio.get_event_loop()
    while True:
        # 修改输入提示，增加选项4
        user_input = await loop.run_in_executor(
            None,
            input,
            "请输入命令（1--识别，2--复位，3--结束，4--拆装操作 或 q 退出）："
        )

        if user_input.strip() == '1':
            cmd = {
                "type": "command_recognition",
                "data": {
                    "duration": 556,
                    "timestamp": int(datetime.now().timestamp()),
                    "uuid": "515ea75e-b7e3-496f-aac1-8109b62859ee"
                }
            }
            try:
                await websocket.send(json.dumps(cmd))
                print("Command sent")
            except websockets.exceptions.ConnectionClosed:
                print("Connection closed, cannot send command.")
                break

        elif user_input.strip() == '2':
            reset_cmd = {
                "type": "command_reset",
                "data": {
                    "duration": 556,
                    "timestamp": int(datetime.now().timestamp()),
                    "uuid": "515ea75e-b7e3-496f-aac1-8109b62859ee"
                }
            }
            try:
                await websocket.send(json.dumps(reset_cmd))
                print("Reset command sent")
            except websockets.exceptions.ConnectionClosed:
                print("Connection closed, cannot send reset command.")
                break

        elif user_input.strip() == '3':
            end_cmd = {
                "type": "command_end",
                "data": {
                    "timestamp": int(datetime.now().timestamp())
                }
            }
            try:
                await websocket.send(json.dumps(end_cmd))
                print("End command sent")
            except websockets.exceptions.ConnectionClosed:
                print("Connection closed, cannot send reset command.")
                break

        # 新增：拆装操作命令
        elif user_input.strip() == '4':
            try:
                duration = int(await loop.run_in_executor(
                    None,
                    input,
                    "请输入拆装操作时长(秒): "
                ))

                assembly_cmd = {
                    "type": "command_assembly",
                    "data": {
                        "duration": duration,
                        "timestamp": datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
                    }
                }
                await websocket.send(json.dumps(assembly_cmd))
                print(f"Assembly command sent (Duration: {duration}s)")

            except ValueError:
                print("错误：请输入有效的数字")
            except websockets.exceptions.ConnectionClosed:
                print("Connection closed, cannot send assembly command.")
                break

        elif user_input.strip().lower() == 'q':
            break


async def main():
    # 启动服务器
    server = await websockets.serve(
        handler,
        "192.168.1.131",
        7272,
        ping_interval=None
    )

    print("Server started at ws://0.0.0.0:7272")
    print("Waiting for connections...")

    # 保持服务器运行
    await server.wait_closed()


if __name__ == "__main__":
    asyncio.run(main())