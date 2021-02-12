/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.fibase.amqp;

import com.fibase.QueueIdentity;
import com.rabbitmq.client.AMQP;
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.Consumer;
import com.rabbitmq.client.DefaultConsumer;
import com.rabbitmq.client.Envelope;
import java.io.IOException;
import java.io.UnsupportedEncodingException;

public class MainReceiveMessage {

    public static void main(String[] argv) throws Exception {
        ConnectionFactory factory = new ConnectionFactory();
        factory.setUsername(QueueIdentity.QUEUEUSERNAME);
        factory.setVirtualHost(QueueIdentity.QUEUEVIRTUALHOST);
        factory.setPassword(QueueIdentity.QUEUEPASSWORD);
        factory.setHost(QueueIdentity.QUEUEHOST);

        Connection connection = factory.newConnection();
        Channel channel = connection.createChannel();

        //DeclareQueue: Ensuring that the queue exists
        channel.queueDeclare(QueueIdentity.QUEUE_NAME, true, false, false, null);
        //Defines the key routes that forward messages to the queue
        channel.queueBind(QueueIdentity.QUEUE_NAME, QueueIdentity.EXCHANGE_NAME, QueueIdentity.ROUTINGKEY);

        //Defines Exchange, responsible for distributing messages in the queues
        channel.exchangeDeclare(QueueIdentity.EXCHANGE_NAME, "direct", true);

        System.out.println(" [*] Waiting for messages. To exit press CTRL+C");

        DataProcessor dp = new DataProcessor();

        Consumer consumer = new DefaultConsumer(channel) {
            public void handleDelivery(String consumerTag, Envelope envelope,
                    AMQP.BasicProperties properties, byte[] body)
                    throws IOException, UnsupportedEncodingException {
                String message = new String(body, "UTF-8");
                message = message.replace("'", "").replace("\n", "").trim();
                System.out.println(" Received " + envelope.getRoutingKey() + ": '" + message + "'");
                dp.process(message);
            }
        };

        channel.basicConsume(QueueIdentity.QUEUE_NAME, true, consumer);

    }
}
