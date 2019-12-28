from flask import Flask, flash, request, redirect, jsonify, render_template, make_response
from werkzeug.exceptions import abort
from forms import ContactForm
from flask_mail import Mail, Message
import json, time, datetime


app = Flask(__name__)
mail = Mail()
app.config.from_object('config')
mail.init_app(app)


@app.route('/index')
@app.route('/')
def index():
    return render_template('index.html')


@app.route('/about')
def about():
    return render_template('about.html')


@app.route('/contact', methods=['GET', 'POST'])
def contact():
    contact_form = ContactForm()

    if request.method == 'POST' and contact_form.validate_on_submit():
        msg = Message('Message from {:s} ({:s})'.format(contact_form.name.data, contact_form.email.data), 
                        sender=contact_form.email.data, reply_to=contact_form.email.data, recipients=[app.config['MAIL_USERNAME']])
        msg.body = '{:s}'.format(contact_form.message.data)
        mail.send(msg)

        flash("<strong>Thank you!</strong> Message sent successfully.")
        return redirect('/contact')

    return render_template('contact.html', form=contact_form)


if __name__ == '__main__':
    app.run(debug=False, host='0.0.0.0')
