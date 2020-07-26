import logging
import time

import serial
from telegram.ext import Updater, CommandHandler, CallbackQueryHandler


logging.basicConfig(format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
                    level=logging.INFO)
logger = logging.getLogger(__name__)


def start(update, context):
    update.message.reply_text('Benvenuto in T_IoT_botGruppo9')
    update.message.reply_text('Comando "/temp" per richiedere la temperatura registrata\nComando "/plus" per aumentare la velocità della ventola\n'
                              'Comando "/minus" per ridurre la velocità della ventola')


def temp(update, context):
    ser = serial.Serial('COM5', 9600)
    time.sleep(2)

    data = []
    for i in range(50):
        b = ser.readline() 
        string_n = b.decode()  
    string = string_n.rstrip()  
    flt = float(string)  
    print(flt)
    data.append(flt) 
    time.sleep(0.1) 

    ser.close()

    for line in data:
        temp = line
        temp = str(temp)

    update.message.reply_text("L'ultima temperatura registrata è " + temp + "°C")
    

def plus(update,context):
    ser = serial.Serial('COM5', 9600)
    val = "+"
    ser.write("+".encode())

def minus(update,context):
    ser = serial.Serial('COM5', 9600)
    val = "-"
    ser.write("-".encode())


def help_command(update, context):
    update.message.reply_text("Use /start to test this bot.")


def main():
    updater = Updater("1178065054:AAF_zEYdJKJX28V0_7FGROgfR6AXHWDUf20", use_context=True)

    updater.dispatcher.add_handler(CommandHandler('start', start))
    updater.dispatcher.add_handler(CommandHandler('temp', temp))
    updater.dispatcher.add_handler(CommandHandler('plus', plus))
    updater.dispatcher.add_handler(CommandHandler('minus', minus))
    updater.dispatcher.add_handler(CommandHandler('help', help_command))

    # Start the Bot
    updater.start_polling()

    updater.idle()


if __name__ == '__main__':
    main()
