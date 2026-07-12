from flask import Flask, flash, request, redirect, jsonify, render_template, make_response, send_from_directory
from werkzeug.exceptions import abort
#from forms import ContactForm
#from flask_mail import Mail, Message
import json, time, datetime


app = Flask(__name__)
#mail = Mail()
#app.config.from_object('config')
#mail.init_app(app)


@app.route('/index')
@app.route('/')
def index():
    return render_template('index.html')


@app.route('/about')
def about():
    return render_template('about.html')

@app.route('/get')
@app.route('/get/<string:version>')
def install(version="26.08"):
    filename = f"Munadi-{version}.run"

    return send_from_directory(app.root_path + '/static/dl/', filename)

@app.route('/.well-known/<path:filename>')
def well_known(filename):
    return send_from_directory(app.root_path + '/.well-known/', filename)

@app.route('/jadid', methods=['GET'])
def jadid():
    if request.method == 'GET':
        v = request.args['v']
        os = request.args['os']
        lang = request.args['lang']

        if os == 'linux':
            return ''
        else:
            return ''

# @app.route('/contact', methods=['GET', 'POST'])
# def contact():
#     contact_form = ContactForm()
#
#     if request.method == 'POST' and contact_form.validate_on_submit():
#         msg = Message('Message from {:s} ({:s})'.format(contact_form.name.data, contact_form.email.data),
#                         sender=contact_form.email.data, reply_to=contact_form.email.data, recipients=[app.config['MAIL_USERNAME']])
#         msg.body = '{:s}'.format(contact_form.message.data)
#         mail.send(msg)
#
#         flash("<strong>Thank you!</strong> Message sent successfully.")
#         return redirect('/contact')
#
#     return render_template('contact.html', form=contact_form)


if __name__ == '__main__':
    app.run(debug=False, host='0.0.0.0')
